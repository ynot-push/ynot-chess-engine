#include "NNUE.h"
#include "Board.h"
#include "BitboardUtils.h"
#include "Piece.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>

std::vector<float> NNUE::featureTransformerWeight;
std::vector<float> NNUE::l1Weight;
std::vector<float> NNUE::l1Bias;
std::vector<float> NNUE::l2Weight;
std::vector<float> NNUE::l2Bias;
std::vector<float> NNUE::outputWeight;
float NNUE::outputBias = 0.0f;
bool  NNUE::loaded = false;

std::vector<int16_t> NNUE::ftWeightQ;

std::vector<int8_t>  NNUE::l1WeightQ;
std::vector<int32_t> NNUE::l1BiasQ;
int32_t NNUE::l1Scale = 1;
int32_t NNUE::l1Shift = 0;

std::vector<int8_t>  NNUE::l2WeightQ;
std::vector<int32_t> NNUE::l2BiasQ;
int32_t NNUE::l2Scale = 1;
int32_t NNUE::l2Shift = 0;

std::vector<int8_t>  NNUE::outputWeightQ;
int32_t NNUE::outputBiasQ = 0;
int32_t NNUE::outputScale = 1;

bool NNUE::load(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;

    char magic[4];
    file.read(magic, 4);
    if (std::memcmp(magic, "NNUE", 4) != 0)
        return false;

    int32_t numFeatures, ftDim, l1Dim, l2Dim;
    file.read(reinterpret_cast<char*>(&numFeatures), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&ftDim), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&l1Dim), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&l2Dim), sizeof(int32_t));

    if (numFeatures != NUM_FEATURES || ftDim != FT_DIM || l1Dim != L1_DIM || l2Dim != L2_DIM)
        return false;

    auto readFloats = [&](std::vector<float>& dst, size_t count)
    {
        dst.resize(count);
        file.read(reinterpret_cast<char*>(dst.data()), count * sizeof(float));
    };

    readFloats(featureTransformerWeight, static_cast<size_t>(NUM_FEATURES + 1) * FT_DIM);
    readFloats(l1Weight, static_cast<size_t>(L1_DIM) * (FT_DIM * 2));
    readFloats(l1Bias, L1_DIM);
    readFloats(l2Weight, static_cast<size_t>(L2_DIM) * L1_DIM);
    readFloats(l2Bias, L2_DIM);
    readFloats(outputWeight, L2_DIM);

    std::vector<float> outBiasVec;
    readFloats(outBiasVec, 1);
    outputBias = outBiasVec[0];

    loaded = static_cast<bool>(file);

    if (loaded)
        quantizeWeights();

    return loaded;
}

bool NNUE::isLoaded() { return loaded; }

int NNUE::orientSquare(bool isWhitePov, int square)
{
    return isWhitePov ? square : (square ^ 0x38);
}

int NNUE::computeFeatureIndex(bool isWhitePov, int square, Piece piece, int orientedKingSquare)
{
    const bool pieceIsWhite = isWhitePiece(piece);
    const int pieceType0 = static_cast<int>(piece) % 6; // 0=pawn..4=queen
    const bool isOurs = (pieceIsWhite == isWhitePov);
    const int pIdx = isOurs ? pieceType0 : pieceType0 + 5;
    const int sqOriented = orientSquare(isWhitePov, square);
    return sqOriented + pIdx * 64 + orientedKingSquare * 640;
}

void NNUE::addFeature(int index, float* __restrict acc)
{
    const float* __restrict row = &featureTransformerWeight[static_cast<size_t>(index) * FT_DIM];
    for (int i = 0; i < FT_DIM; i++)
        acc[i] += row[i];
}

void NNUE::removeFeature(int index, float* __restrict acc)
{
    const float* __restrict row = &featureTransformerWeight[static_cast<size_t>(index) * FT_DIM];
    for (int i = 0; i < FT_DIM; i++)
        acc[i] -= row[i];
}

void NNUE::collectHalfKPFeatures(const Board& board, bool isWhitePov, std::vector<int>& outIndices)
{
    outIndices.clear();

    const Bitboard kingBB = isWhitePov ? board.getWhiteKing() : board.getBlackKing();
    const int kingSquare = BitboardUtils::getLSB(kingBB);
    const int kingOriented = orientSquare(isWhitePov, kingSquare);

    for (int square = 0; square < 64; square++)
    {
        const Piece piece = board.getPiece(square);
        if (piece == EMPTY || piece == WKING || piece == BKING)
            continue;

        outIndices.push_back(computeFeatureIndex(isWhitePov, square, piece, kingOriented));
    }
}

int NNUE::collectHalfKPFeaturesFixed(const Board& board, bool isWhitePov, int outIndices[32])
{
    const Bitboard kingBB = isWhitePov ? board.getWhiteKing() : board.getBlackKing();
    const int kingSquare = BitboardUtils::getLSB(kingBB);
    const int kingOriented = orientSquare(isWhitePov, kingSquare);

    int count = 0;
    for (int square = 0; square < 64; square++)
    {
        const Piece piece = board.getPiece(square);
        if (piece == EMPTY || piece == WKING || piece == BKING)
            continue;

        outIndices[count++] = computeFeatureIndex(isWhitePov, square, piece, kingOriented);
    }
    return count;
}

void NNUE::accumulate(const Board& board, bool isWhitePov, float out[FT_DIM])
{
    std::fill(out, out + FT_DIM, 0.0f);

    std::vector<int> features;
    collectHalfKPFeatures(board, isWhitePov, features);

    for (int idx : features)
        addFeature(idx, out);
}

void NNUE::refreshAccumulator(const Board& board, bool isWhitePov, float out[FT_DIM])
{
    accumulate(board, isWhitePov, out);
}

int NNUE::evaluateFromAccumulators(const float* __restrict us, const float* __restrict them)
{
    alignas(32) float x[FT_DIM * 2];
    for (int i = 0; i < FT_DIM; i++)
    {
        x[i]          = std::clamp(us[i], 0.0f, 1.0f);
        x[FT_DIM + i] = std::clamp(them[i], 0.0f, 1.0f);
    }

    alignas(32) float h1[L1_DIM];
    const float* __restrict l1W = l1Weight.data();
    const float* __restrict l1B = l1Bias.data();
    for (int o = 0; o < L1_DIM; o++)
    {
        float sum = l1B[o];
        const float* __restrict row = l1W + static_cast<size_t>(o) * (FT_DIM * 2);
        for (int i = 0; i < FT_DIM * 2; i++)
            sum += row[i] * x[i];
        h1[o] = std::clamp(sum, 0.0f, 1.0f);
    }

    alignas(32) float h2[L2_DIM];
    const float* __restrict l2W = l2Weight.data();
    const float* __restrict l2B = l2Bias.data();
    for (int o = 0; o < L2_DIM; o++)
    {
        float sum = l2B[o];
        const float* __restrict row = l2W + static_cast<size_t>(o) * L1_DIM;
        for (int i = 0; i < L1_DIM; i++)
            sum += row[i] * h1[i];
        h2[o] = std::clamp(sum, 0.0f, 1.0f);
    }

    const float* __restrict outW = outputWeight.data();
    float raw = outputBias;
    for (int i = 0; i < L2_DIM; i++)
        raw += outW[i] * h2[i];

    return static_cast<int>(raw * K);
}

int NNUE::evaluate(const Board& board)
{
    const bool usIsWhite = board.isWhiteToMove();

    float us[FT_DIM];
    float them[FT_DIM];
    accumulate(board, usIsWhite, us);
    accumulate(board, !usIsWhite, them);

    return evaluateFromAccumulators(us, them);
}
int32_t NNUE::pickInt8Scale(const std::vector<float>& weights)
{
    float maxAbs = 0.0f;
    for (float w : weights)
        maxAbs = std::max(maxAbs, std::abs(w));

    if (maxAbs < 1e-8f)
        return 64; 
    int32_t scale = 1;
    while (scale * 2 <= 4096 && maxAbs * static_cast<float>(scale * 2) <= 126.0f)
        scale *= 2;

    return scale;
}

void NNUE::quantizeWeights()
{
    ftWeightQ.resize(featureTransformerWeight.size());
    for (size_t i = 0; i < featureTransformerWeight.size(); i++)
    {
        float scaled = featureTransformerWeight[i] * static_cast<float>(ACT_SCALE);
        scaled = std::clamp(scaled, -32768.0f, 32767.0f);
        ftWeightQ[i] = static_cast<int16_t>(std::lround(scaled));
    }

    auto quantizeLayer = [](const std::vector<float>& weights, const std::vector<float>& bias,
                             std::vector<int8_t>& weightsQ, std::vector<int32_t>& biasQ,
                             int32_t& scaleOut, int32_t& shiftOut)
    {
        scaleOut = pickInt8Scale(weights);
        shiftOut = 0;
        for (int32_t s = scaleOut; s > 1; s >>= 1)
            shiftOut++;

        weightsQ.resize(weights.size());
        for (size_t i = 0; i < weights.size(); i++)
        {
            float scaled = weights[i] * static_cast<float>(scaleOut);
            scaled = std::clamp(scaled, -127.0f, 127.0f);
            weightsQ[i] = static_cast<int8_t>(std::lround(scaled));
        }

        biasQ.resize(bias.size());
        const double biasScale = static_cast<double>(ACT_SCALE) * static_cast<double>(scaleOut);
        for (size_t i = 0; i < bias.size(); i++)
            biasQ[i] = static_cast<int32_t>(std::lround(static_cast<double>(bias[i]) * biasScale));
    };

    quantizeLayer(l1Weight, l1Bias, l1WeightQ, l1BiasQ, l1Scale, l1Shift);
    quantizeLayer(l2Weight, l2Bias, l2WeightQ, l2BiasQ, l2Scale, l2Shift);
    std::vector<float> outputBiasVec{ outputBias };
    std::vector<int32_t> outputBiasQVec;
    int32_t unusedShift = 0;
    quantizeLayer(outputWeight, outputBiasVec, outputWeightQ, outputBiasQVec, outputScale, unusedShift);
    outputBiasQ = outputBiasQVec[0];
}

void NNUE::addFeatureInt(int index, int32_t* __restrict acc)
{
    const int16_t* __restrict row = &ftWeightQ[static_cast<size_t>(index) * FT_DIM];
    for (int i = 0; i < FT_DIM; i++)
        acc[i] += row[i];
}

void NNUE::removeFeatureInt(int index, int32_t* __restrict acc)
{
    const int16_t* __restrict row = &ftWeightQ[static_cast<size_t>(index) * FT_DIM];
    for (int i = 0; i < FT_DIM; i++)
        acc[i] -= row[i];
}

void NNUE::refreshAccumulatorInt(const Board& board, bool isWhitePov, int32_t out[FT_DIM])
{
    std::fill(out, out + FT_DIM, 0);

    int features[32];
    const int count = collectHalfKPFeaturesFixed(board, isWhitePov, features);

    for (int i = 0; i < count; i++)
        addFeatureInt(features[i], out);
}

#if defined(__AVX2__)
#include <immintrin.h>

namespace
{
    inline int32_t hsumEpi32Avx2(__m256i v)
    {
        __m128i lo = _mm256_castsi256_si128(v);
        __m128i hi = _mm256_extracti128_si256(v, 1);
        __m128i sum128 = _mm_add_epi32(lo, hi);
        __m128i shuf = _mm_shuffle_epi32(sum128, _MM_SHUFFLE(1, 0, 3, 2));
        sum128 = _mm_add_epi32(sum128, shuf);
        shuf = _mm_shuffle_epi32(sum128, _MM_SHUFFLE(0, 1, 0, 1));
        sum128 = _mm_add_epi32(sum128, shuf);
        return _mm_cvtsi128_si32(sum128);
    }
    inline int32_t dotProductU8I8(const uint8_t* __restrict a, const int8_t* __restrict b, int n)
    {
        __m256i acc = _mm256_setzero_si256();
        const __m256i ones16 = _mm256_set1_epi16(1);

        int i = 0;
        for (; i + 32 <= n; i += 32)
        {
            const __m256i av = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
            const __m256i bv = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + i));
            const __m256i prod16 = _mm256_maddubs_epi16(av, bv);
            const __m256i prod32 = _mm256_madd_epi16(prod16, ones16);
            acc = _mm256_add_epi32(acc, prod32);
        }

        int32_t sum = hsumEpi32Avx2(acc);
        for (; i < n; i++)
            sum += static_cast<int32_t>(a[i]) * static_cast<int32_t>(b[i]);
        return sum;
    }
}
#else
namespace
{
    inline int32_t dotProductU8I8(const uint8_t* __restrict a, const int8_t* __restrict b, int n)
    {
        int32_t sum = 0;
        for (int i = 0; i < n; i++)
            sum += static_cast<int32_t>(a[i]) * static_cast<int32_t>(b[i]);
        return sum;
    }
}
#endif

int NNUE::evaluateFromAccumulatorsInt(const int32_t* __restrict us, const int32_t* __restrict them)
{
    alignas(32) uint8_t x[FT_DIM * 2];
    for (int i = 0; i < FT_DIM; i++)
    {
        x[i]          = static_cast<uint8_t>(std::clamp(us[i],   0, ACT_SCALE));
        x[FT_DIM + i] = static_cast<uint8_t>(std::clamp(them[i], 0, ACT_SCALE));
    }

    alignas(32) uint8_t h1[L1_DIM];
    const int8_t*  __restrict l1W = l1WeightQ.data();
    const int32_t* __restrict l1B = l1BiasQ.data();
    for (int o = 0; o < L1_DIM; o++)
    {
        const int8_t* __restrict row = l1W + static_cast<size_t>(o) * (FT_DIM * 2);
        const int32_t sum = l1B[o] + dotProductU8I8(x, row, FT_DIM * 2);
        const int32_t shifted = sum >> l1Shift;
        h1[o] = static_cast<uint8_t>(std::clamp(shifted, 0, ACT_SCALE));
    }

    alignas(32) uint8_t h2[L2_DIM];
    const int8_t*  __restrict l2W = l2WeightQ.data();
    const int32_t* __restrict l2B = l2BiasQ.data();
    for (int o = 0; o < L2_DIM; o++)
    {
        const int8_t* __restrict row = l2W + static_cast<size_t>(o) * L1_DIM;
        const int32_t sum = l2B[o] + dotProductU8I8(h1, row, L1_DIM);
        const int32_t shifted = sum >> l2Shift;
        h2[o] = static_cast<uint8_t>(std::clamp(shifted, 0, ACT_SCALE));
    }

    const int8_t* __restrict outW = outputWeightQ.data();
    const int64_t raw = static_cast<int64_t>(outputBiasQ) + dotProductU8I8(h2, outW, L2_DIM);
    const int64_t scaledEval = (raw * static_cast<int64_t>(K)) /
                                (static_cast<int64_t>(ACT_SCALE) * static_cast<int64_t>(outputScale));

    return static_cast<int>(scaledEval);
}