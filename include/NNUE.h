#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "PieceType.h"

class Board;

class NNUE
{
public:
    static constexpr int FT_DIM = 256;

    static bool load(const std::string& path);
    static bool isLoaded();

    // Float reference path (used as ground truth, e.g. by AccumulatorVerifyTest)
    static int evaluate(const Board& board);

    static void refreshAccumulatorInt(const Board& board, bool isWhitePov, int32_t out[FT_DIM]);
    static int evaluateFromAccumulatorsInt(const int32_t* __restrict us, const int32_t* __restrict them);
    static void addFeatureInt(int index, int32_t* __restrict acc);
    static void removeFeatureInt(int index, int32_t* __restrict acc);

    static int computeFeatureIndex(bool isWhitePov, int square, Piece piece, int orientedKingSquare);
    static int orientSquare(bool isWhitePov, int square);

private:
    static constexpr int NUM_FEATURES = 40960;
    static constexpr int L1_DIM       = 32;
    static constexpr int L2_DIM       = 32;
    static constexpr float K          = 600.0f;
    static std::vector<float> featureTransformerWeight;
    static std::vector<float> l1Weight;
    static std::vector<float> l1Bias;
    static std::vector<float> l2Weight;
    static std::vector<float> l2Bias;
    static std::vector<float> outputWeight;
    static float outputBias;
    static bool loaded;

    static void accumulate(const Board& board, bool isWhitePov, float out[FT_DIM]);
    static void addFeature(int index, float* __restrict acc);
    static void removeFeature(int index, float* __restrict acc);
    static void refreshAccumulator(const Board& board, bool isWhitePov, float out[FT_DIM]);
    static int evaluateFromAccumulators(const float* __restrict us, const float* __restrict them);
    static void collectHalfKPFeatures(const Board& board, bool isWhitePov, std::vector<int>& outIndices);
    static int collectHalfKPFeaturesFixed(const Board& board, bool isWhitePov, int outIndices[32]);
    static constexpr int32_t ACT_SCALE = 127;

    static std::vector<int16_t> ftWeightQ;   // scale ACT_SCALE

    static std::vector<int8_t>  l1WeightQ;   // scale l1Scale (power of two)
    static std::vector<int32_t> l1BiasQ;     // scale ACT_SCALE * l1Scale
    static int32_t l1Scale;
    static int32_t l1Shift;                  // log2(l1Scale)

    static std::vector<int8_t>  l2WeightQ;   // scale l2Scale
    static std::vector<int32_t> l2BiasQ;     // scale ACT_SCALE * l2Scale
    static int32_t l2Scale;
    static int32_t l2Shift;

    static std::vector<int8_t>  outputWeightQ; // scale outputScale
    static int32_t outputBiasQ;                // scale ACT_SCALE * outputScale
    static int32_t outputScale;

    static void quantizeWeights();
    static int32_t pickInt8Scale(const std::vector<float>& weights);
};