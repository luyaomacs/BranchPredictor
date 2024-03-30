//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

uint32_t gmask;
uint32_t gsize;

// Gshare: global history based on index sharing
uint32_t ghr;              // GHR: Global history register
uint8_t *pht;              // BHT: key=index, val=counter (SN, WN, WT, ST)

// Tournament
uint8_t *choicePredictors; // choose which predictor (l/g) to use
uint8_t *lPredictors;
uint32_t *lhistoryTable;
uint32_t gpred_cnt;

// Custom
typedef struct {
  int8_t *weights; // Weights are signed integers
} Perceptron;

Perceptron *perceptronTable;
int tableSize;

//-------------------------------------//
//    Predictor Function Declaration   //
//-------------------------------------//

void gshare_init();
void tournament_init();
void custom_init();

uint8_t gshare_pred();
uint8_t tournament_pred();
uint8_t custom_pred();

void gshare_train(uint32_t pc, uint8_t outcome);
void tournament_train(uint32_t pc, uint8_t outcome);
void custom_train(uint32_t pc, uint8_t outcome);

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  gsize = 1 << ghistoryBits;
  gmask = gsize - 1;

  switch (bpType) {
    case GSHARE:
      return gshare_init();
    case TOURNAMENT:
      return tournament_init();
    case CUSTOM:
      return custom_init();
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_pred(pc);
    case TOURNAMENT:
      return tournament_pred(pc);
    case CUSTOM:
      return custom_pred(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType) {
    case GSHARE:
      return gshare_train(pc, outcome);
    case TOURNAMENT:
      return tournament_train(pc, outcome);
    case CUSTOM:
      return custom_train(pc, outcome);
    default:
      break;
  }
}

void
gshare_init() {
  ghr = 0; // Initialize to NT

  pht = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(pht, 1, gsize); // Weakly not taken (WN)
}

void
tournament_init() {
  uint32_t lsize = 1 << lhistoryBits;

  ghr = 0; // Initialize to NT
  gpred_cnt = 0;

  pht = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(pht, 1, gsize); // Weakly not taken (WN)

  choicePredictors = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(choicePredictors, 2, gsize); // Initialize to weekly select global predictor

  lPredictors = (uint8_t *)malloc(sizeof(uint8_t) * lsize);
  memset(lPredictors, 1, lsize); // Weakly not taken (WN)

  lhistoryTable = (uint32_t *)calloc(1 << pcIndexBits, sizeof(uint32_t));
}

void
custom_init() {
  // Assuming ghistoryBits has been set globally
  ghr = 0; // Start with an empty history

  tableSize = (1 << pcIndexBits); // 2^pcIndexBits perceptrons
  perceptronTable = (Perceptron *)malloc(tableSize * sizeof(Perceptron));
  
  for (int i = 0; i < tableSize; i++) {
    perceptronTable[i].weights = (int8_t *)malloc((ghistoryBits + 1) * sizeof(int8_t));
    for (int j = 0; j < ghistoryBits + 1; j++) {
      perceptronTable[i].weights[j] = 0; // Initialize weights to zero
    }
  }
}

uint8_t
gshare_pred(uint32_t pc) {
  uint32_t index = (pc ^ ghr) & gmask;
  return pht[index] >> (COUNTER - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
tournament_pred(uint32_t pc) {
  uint8_t predict = NOTTAKEN;
  uint8_t selector = choicePredictors[ghr] >> (COUNTER - 1);
  if (selector) {
    predict = pht[ghr];
    gpred_cnt++;
  } else {
    predict = lPredictors[lhistoryTable[pc & ((1 << pcIndexBits) - 1)]];
  }
  return predict >> (COUNTER - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
custom_pred(uint32_t pc) {
  // Hash function to map PC to perceptron table index, ensure index is within the table size
  int index = (pc ^ ghr) % tableSize;
  
  // Compute the dot product of GHR and perceptron weights
  int sum = perceptronTable[index].weights[0]; // Start with the bias weight
  for (int i = 1; i <= ghistoryBits; i++) {
    int bit = (ghr >> (ghistoryBits - i)) & 1; // Extract the ith history bit
    bit = (bit << 1) - 1; // Convert to -1 or 1
    sum += bit * perceptronTable[index].weights[i];
  }

//   printf("Sum: %d\n", sum);
  return sum>=0? TAKEN : NOTTAKEN;

}

void
gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t index = (pc ^ ghr) & gmask;
  ghr = (ghr << 1 | outcome) & gmask;
  if (outcome) {
    pht[index] += pht[index] == (1 << COUNTER) - 1 ? 0 : 1;
  } else {
    pht[index] -= pht[index] == 0 ? 0 : 1;
  }
}

void
tournament_train(uint32_t pc, uint8_t outcome) {
  uint8_t MAX = (1 << COUNTER) - 1;
  uint32_t pcindex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lindex = lhistoryTable[pcindex];

  uint8_t gCorrect = pht[ghr] >> (COUNTER - 1) == outcome;
  uint8_t lCorrect = lPredictors[lindex] >> (COUNTER - 1) == outcome;

  int action = gCorrect - lCorrect;
  if (action > 0) {
    choicePredictors[ghr] += choicePredictors[ghr] == MAX ? 0 : 1;
  } else if (action < 0) {
    choicePredictors[ghr] -= choicePredictors[ghr] == 0 ? 0 : 1;
  }

  if (outcome) {
    pht[ghr] += pht[ghr] == MAX ? 0 : 1;
    lPredictors[lindex] += lPredictors[lindex] == MAX ? 0 : 1;
  } else {
    pht[ghr] -= pht[ghr] == 0 ? 0 : 1;
    lPredictors[lindex] -= lPredictors[lindex] == 0 ? 0 : 1;
  }

  ghr = (ghr << 1 | outcome) & gmask;
  lhistoryTable[pcindex] = (lhistoryTable[pcindex] << 1 | outcome) & ((1 << lhistoryBits) - 1);
}

void
custom_train(uint32_t pc, uint8_t outcome) {
  int index = (pc ^ ghr) % tableSize;

  int sum = 0; // We will recompute the dot product for the training
  for (int i = 1; i <= ghistoryBits; i++) {
    int bit = (ghr >> (ghistoryBits - i)) & 1;
    bit = (bit << 1) - 1;
    sum += bit * perceptronTable[index].weights[i];
  }
  sum += perceptronTable[index].weights[0]; // Add the bias weight

  int prediction = (sum >= 0) ? TAKEN : NOTTAKEN;
  int target = (outcome == TAKEN) ? 1 : -1; // Convert outcome to +1/-1 for training

  // Train the perceptron if the prediction was incorrect or sum is within a threshold
//   int THRESHOLD = 1.93 * ghistoryBits + 14;
  int THRESHOLD = 1.93 * ghistoryBits + 14;
  if ((prediction != outcome) || (abs(sum) < THRESHOLD)) {
    perceptronTable[index].weights[0] += target; // Update bias weight
    for (int i = 1; i <= ghistoryBits; i++) {
      int bit = (ghr >> (ghistoryBits - i)) & 1;
      bit = (bit << 1) - 1;
      perceptronTable[index].weights[i] += target * bit; // Update weights
    }
  }

  // Update GHR with the new outcome
  ghr = ((ghr << 1) | (outcome == TAKEN)) & gmask;

}

void
wrap_up_predictor() {
  switch (bpType) {
    case GSHARE: {
      free(pht);
      break;
    }
    case TOURNAMENT: {
      free(pht);
      free(choicePredictors);
      free(lPredictors);
      free(lhistoryTable);
      break;
    }
    case CUSTOM: {
      free(pht);
      free(perceptronTable);
      break;
    }
    default:
      break;
  }
}