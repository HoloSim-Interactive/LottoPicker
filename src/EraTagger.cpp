#include "lottopicker/EraTagger.h"

namespace lottopicker {

void EraTagger::tag(std::vector<DrawRecord> &records, const std::vector<EraBoundary> &eras) {
    for (DrawRecord &record : records) {
        record.poolSize = poolSizeForDate(record.date, eras);
    }
}

} // namespace lottopicker
