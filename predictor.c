//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "LuyaoMa";
const char *studentID   = "A59023451";
const char *email       = "luma@ucsd.edu";

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

//
//TODO: Add your own Branch Predictor data structures here
//
// Gshare
uint32_t GHR = 0;      // Global History Register
uint8_t PHT[1 << 32];  // Pattern History Table with length 32

//Tournament


//Custom


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  
  // Choose initialization based on the bpType
  switch (bpType) {
    case GSHARE:
      return init_gshare();
    case TOURNAMENT:
      return init_tournament();
    case CUSTOM:
      return init_custom();
    default:
      break;
  }

  GHR = 0;
  int pht_entries = 1 << ghistoryBits;
  for (int i = 0; i < pht_entries; i++) {
    PHT[i] = WN;  // Initialize to weakly not taken
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
      return pred_gshare(pc);
      // uint32_t pht_index = (pc ^ GHR) & ((1 << ghistoryBits) - 1);
      // uint8_t prediction = PHT[pht_index];
      // return (prediction >= WT) ? TAKEN : NOTTAKEN;
    case TOURNAMENT:
      return pred_tournament(pc);
    case CUSTOM:
      return pred_custom(pc);
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
  //
  //TODO: Implement Predictor training
  //
}
