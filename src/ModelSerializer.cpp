#include "lottopicker/ModelSerializer.h"

#include <array>
#include <charconv>
#include <fstream>
#include <string>
#include <vector>

#include "lottopicker/CooccurrenceScorer.h" // kMinGroupSize/kMaxGroupSize
#include "lottopicker/Errors.h"

namespace lottopicker {

namespace {

constexpr const char *kHeaderLine = "LOTTOPICKER_MODEL v2";
constexpr const char *kSourceHashPrefix = "source_hash=";
constexpr const char *kDateRangePrefix = "date_range=";
constexpr const char *kDrawCountPrefix = "draw_count=";
constexpr const char *kBaselineCoocPrefix = "baseline_cooc=";
constexpr const char *kPerNumberSection = "[per_number]";
constexpr const char *kGroupSectionPrefix = "[group_scores:";

std::string groupSectionHeader(int groupSize) {
    return std::string(kGroupSectionPrefix) + std::to_string(groupSize) + "]";
}

// Shortest-round-trip textual form of `value` (docs/SDD.md's
// documented precision requirement for TP-DATA-OUT-301's bit-for-bit
// clause).
std::string formatDouble(double value) {
    std::array<char, 64> buf{};
    const auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
    return std::string(buf.data(), result.ptr);
}

double parseDouble(const std::string &token, const std::string &context) {
    double value = 0.0;
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
    if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
        throw ModelStoreError("model artifact: invalid numeric value '" + token + "' in " +
                              context);
    }
    return value;
}

int parseInt(const std::string &token, const std::string &context) {
    int value = 0;
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
    if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
        throw ModelStoreError("model artifact: invalid integer '" + token + "' in " + context);
    }
    return value;
}

// Splits `line` on ',' into fields, preserving empty trailing fields
// (so e.g. "1,2,3.5" -> {"1","2","3.5"}).
std::vector<std::string> splitCsv(const std::string &line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (start <= line.size()) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
    return fields;
}

} // namespace

void ModelSerializer::write(const ModelArtifact &artifact, const std::filesystem::path &path) {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        throw ModelStoreError("cannot open model artifact for writing: " + path.string());
    }

    out << kHeaderLine << "\n";
    out << kSourceHashPrefix << artifact.sourceHash << "\n";
    out << kDateRangePrefix << artifact.earliestDate << "," << artifact.latestDate << "\n";
    out << kDrawCountPrefix << artifact.drawCount << "\n";

    out << kBaselineCoocPrefix;
    for (std::size_t i = 0; i < artifact.baselineCooc.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << formatDouble(artifact.baselineCooc[i]);
    }
    out << "\n";

    out << kPerNumberSection << "\n";
    for (const auto &[number, score] : artifact.perNumber) {
        out << number << "," << formatDouble(score) << "\n";
    }

    for (int groupSize = kMinGroupSize; groupSize <= kMaxGroupSize; ++groupSize) {
        out << groupSectionHeader(groupSize) << "\n";
        const auto it = artifact.groupScores.find(groupSize);
        if (it == artifact.groupScores.end()) {
            continue;
        }
        for (const auto &[group, score] : it->second) {
            for (int number : group) {
                out << number << ",";
            }
            out << formatDouble(score) << "\n";
        }
    }

    if (!out) {
        throw ModelStoreError("failed writing model artifact: " + path.string());
    }
}

ModelArtifact ModelSerializer::read(const std::filesystem::path &path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw ModelStoreError("cannot open model artifact for reading: " + path.string());
    }

    // Same CRLF tolerance as Config::parse.
    const auto nextLine = [&in](std::string &line) -> bool {
        if (!std::getline(in, line)) {
            return false;
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return true;
    };

    std::string line;
    if (!nextLine(line) || line != kHeaderLine) {
        throw ModelStoreError("model artifact: missing or unrecognized header in " + path.string());
    }

    ModelArtifact artifact;

    if (!nextLine(line) || line.rfind(kSourceHashPrefix, 0) != 0) {
        throw ModelStoreError("model artifact: missing source_hash in " + path.string());
    }
    artifact.sourceHash = line.substr(std::char_traits<char>::length(kSourceHashPrefix));

    if (!nextLine(line) || line.rfind(kDateRangePrefix, 0) != 0) {
        throw ModelStoreError("model artifact: missing date_range in " + path.string());
    }
    {
        const std::string value = line.substr(std::char_traits<char>::length(kDateRangePrefix));
        const std::size_t comma = value.find(',');
        if (comma == std::string::npos) {
            throw ModelStoreError("model artifact: malformed date_range in " + path.string());
        }
        artifact.earliestDate = value.substr(0, comma);
        artifact.latestDate = value.substr(comma + 1);
    }

    if (!nextLine(line) || line.rfind(kDrawCountPrefix, 0) != 0) {
        throw ModelStoreError("model artifact: missing draw_count in " + path.string());
    }
    artifact.drawCount = static_cast<std::size_t>(
        parseInt(line.substr(std::char_traits<char>::length(kDrawCountPrefix)), "draw_count"));

    if (!nextLine(line) || line.rfind(kBaselineCoocPrefix, 0) != 0) {
        throw ModelStoreError("model artifact: missing baseline_cooc in " + path.string());
    }
    {
        const std::vector<std::string> fields =
            splitCsv(line.substr(std::char_traits<char>::length(kBaselineCoocPrefix)));
        if (fields.size() != kGroupSizeCount) {
            throw ModelStoreError("model artifact: malformed baseline_cooc in " + path.string());
        }
        for (std::size_t i = 0; i < fields.size(); ++i) {
            artifact.baselineCooc[i] = parseDouble(fields[i], "baseline_cooc");
        }
    }

    if (!nextLine(line) || line != kPerNumberSection) {
        throw ModelStoreError("model artifact: missing [per_number] section in " + path.string());
    }

    // [per_number] rows continue until the first "[group_scores:"
    // section header or end of file.
    bool haveLine = nextLine(line);
    while (haveLine && line.rfind(kGroupSectionPrefix, 0) != 0) {
        const std::vector<std::string> fields = splitCsv(line);
        if (fields.size() != 2) {
            throw ModelStoreError("model artifact: malformed per_number row '" + line + "'");
        }
        const int number = parseInt(fields[0], "per_number number");
        artifact.perNumber[number] = parseDouble(fields[1], "per_number score");
        haveLine = nextLine(line);
    }

    for (int groupSize = kMinGroupSize; groupSize <= kMaxGroupSize; ++groupSize) {
        const std::string expectedHeader = groupSectionHeader(groupSize);
        if (!haveLine || line != expectedHeader) {
            throw ModelStoreError("model artifact: expected section '" + expectedHeader + "' in " +
                                  path.string());
        }
        haveLine = nextLine(line);
        while (haveLine && line.rfind(kGroupSectionPrefix, 0) != 0) {
            const std::vector<std::string> fields = splitCsv(line);
            if (static_cast<int>(fields.size()) != groupSize + 1) {
                throw ModelStoreError("model artifact: malformed group_scores row '" + line + "'");
            }
            GroupKey group;
            group.reserve(static_cast<std::size_t>(groupSize));
            for (int i = 0; i < groupSize; ++i) {
                group.push_back(
                    parseInt(fields[static_cast<std::size_t>(i)], "group_scores number"));
            }
            const double score =
                parseDouble(fields[static_cast<std::size_t>(groupSize)], "group_scores score");
            artifact.groupScores[groupSize][group] = score;
            haveLine = nextLine(line);
        }
    }

    return artifact;
}

std::optional<ModelArtifact> ModelSerializer::tryRead(const std::filesystem::path &path) {
    try {
        return read(path);
    } catch (const ModelStoreError &) {
        return std::nullopt;
    }
}

} // namespace lottopicker
