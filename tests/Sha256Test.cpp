#include "lottopicker/Sha256.h"

#include <catch2/catch_test_macros.hpp>

using lottopicker::Sha256;

// Standard published SHA-256 test vectors (FIPS 180-4 / NIST examples)
// -- confirms the hand-rolled implementation matches a well-known
// reference, independent of anything CORE-204-specific.
TEST_CASE("Sha256 matches the published test vector for an empty string", "[CORE-204]") {
    CHECK(Sha256::hexDigest("") ==
          "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_CASE("Sha256 matches the published test vector for \"abc\"", "[CORE-204]") {
    CHECK(Sha256::hexDigest("abc") ==
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_CASE("Sha256 matches the published test vector spanning multiple 512-bit blocks",
          "[CORE-204]") {
    // NIST's two-block message test vector.
    CHECK(Sha256::hexDigest("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("Sha256 is deterministic and content-sensitive", "[CORE-204]") {
    const std::string digestA =
        Sha256::hexDigest("date,n1,n2,n3,n4,n5,n6\n2020-01-01,1,2,3,4,5,6\n");
    const std::string digestB =
        Sha256::hexDigest("date,n1,n2,n3,n4,n5,n6\n2020-01-01,1,2,3,4,5,6\n");
    const std::string digestC =
        Sha256::hexDigest("date,n1,n2,n3,n4,n5,n6\n2020-01-01,1,2,3,4,5,7\n"); // one digit differs

    CHECK(digestA == digestB);
    CHECK(digestA != digestC);
    CHECK(digestA.size() == 64);
}
