//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>

//
// TODO:Student Information
//
const char *studentName = "Tongchuan Shen";
const char *studentID   = "A13976009";
const char *email       = "toshen@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[5] = { "Static", "Gshare",
                          "Tournament", "Custom","Comb" };

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
int number_of_states;
int global_history;         // Global history register (GHR)
int * gprediction_table;    // Global prediction (GPT)
int ghistory_mask;


int * lhistory_table;       // Local history table (LHT)
int * lprediction_table;    // Local prediction (LPT)
int * choice_prediction;    // Choice prediction (CPT)
int pc_mask;
int lhistory_mask;


int patternBits;
int * pattern_table;
int * history_table;

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
  global_history = 0;
  printf("Init btype: %d",bpType);
  switch(bpType){
    case GSHARE: {
      printf(" GSHARE\n");
      ghistory_mask = (1 << ghistoryBits) -1;
      number_of_states = 1 << ghistoryBits;   //pow(2,ghistorybit-1) 
      gprediction_table =  (int*) malloc(sizeof(int) * number_of_states);
      for (int i=0;i<number_of_states;i++){
        gprediction_table[i] = 0;
      }
    }
    case TOURNAMENT: {
      printf(" TOURNAMENT\n");
      ghistory_mask = (1 << ghistoryBits) -1;
      pc_mask = (1 << pcIndexBits) -1;
      lhistory_mask = (1 << lhistoryBits) -1;
      lhistory_table = (int*) malloc(sizeof(int) * (1 << lhistoryBits));
      lprediction_table = (int*) malloc(sizeof(int) * (1 << pcIndexBits));
      gprediction_table =  (int*) malloc(sizeof(int) * (1 << ghistoryBits));
      choice_prediction = (int*) malloc(sizeof(int) * (1 << ghistoryBits));

      for (int i=0;i<(1 << lhistoryBits);i++){
        lhistory_table[i] = 0;
      }
      for (int i=0;i<(1 << ghistoryBits);i++){
        gprediction_table[i] = WN;
        choice_prediction[i] = WN;
      }
      for (int i=0;i<(1 << pcIndexBits);i++){
        lprediction_table[i] = 0;
      }
      return;
    }
    case CUSTOM:{
      ghistoryBits = 13;
      lhistoryBits = 11;
      patternBits = 11;
      pcIndexBits = 11;
      ghistory_mask = (1 << ghistoryBits) -1;

      pattern_table = (int*)malloc(sizeof(int)*(1<<patternBits));
      history_table = (int*)malloc(sizeof(int) * (1 << lhistoryBits));
      choice_prediction = (int*) malloc(sizeof(int) * (1 << ghistoryBits));

      gprediction_table =  (int*) malloc(sizeof(int) * (1 << ghistoryBits));

      for (int i=0;i<(1<<patternBits);i++){
        pattern_table[i] = 1;
      }

      for (int i=0;i<(1<<lhistoryBits);i++){
        history_table[i] = 1;
      }
      
      for (int i=0;i<(1 << ghistoryBits);i++){
        gprediction_table[i] = WN;
        choice_prediction[i] = WN;
      }
    }
    case 4:{
      
    }
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
    case GSHARE:{
      int history = global_history & ghistory_mask;
      int address = pc & ghistory_mask;
      int prediction = gprediction_table[history ^ address];
      if (prediction >= 2)
        return TAKEN;
      else
        return NOTTAKEN;
    }
      
    case TOURNAMENT:{
      int history_idx = global_history & ghistory_mask;
      int choice = choice_prediction[history_idx];
      // local predictor
      int lhist_idx = pc & pc_mask;
      int lhist_val = lhistory_table[lhist_idx];
      int local_prediction = lprediction_table[lhist_val];
      int local_result = TAKEN;
      if (local_prediction == WN || local_prediction == SN) {
        local_result = NOTTAKEN;
      }
      // global predictor
      int history = global_history & ghistory_mask;
      int address = pc & ghistory_mask;
      int prediction = gprediction_table[history ^ address];
      int global_result = NOTTAKEN;
      if (prediction >= 2)
        global_result = TAKEN;
      // choose between local & global
      if (choice == SN || choice == WN) {
        return local_result;
      } else {
        return global_result;
      }
    }

    case CUSTOM:{
      int history_idx = global_history & ghistory_mask;
      int choice = choice_prediction[history_idx];
      // local predictor
      int index_mask = (1<<lhistoryBits) -1;
      int history_table_index = pc & index_mask;
      int pattern_index = history_table[history_table_index];
      int local_result = NOTTAKEN;
      if (pattern_table[pattern_index]>1){
        local_result = TAKEN;
      }
      
      // global predictor
      int history = global_history & ghistory_mask;
      int address = pc & ghistory_mask;
      int prediction = gprediction_table[history ^ address];
      int global_result = NOTTAKEN;
      if (prediction >= 2)
        global_result = TAKEN;
      // choose between local & global
      if (choice == SN || choice == WN) {
        return local_result;
      } else {
        return global_result;
      }
    }
    case 4:{
      
    }
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
  switch (bpType)
  {
  case GSHARE:{
    int table_index = (global_history & ghistory_mask) ^ (pc & ghistory_mask);
    if (outcome == NOTTAKEN){
      if (gprediction_table[table_index]>0)
        gprediction_table[table_index]--;
      global_history = global_history << 1;
    }
    else{
      if (gprediction_table[table_index]<3){
        gprediction_table[table_index]++;
      }
      global_history = (global_history << 1) + 1;
    }
  }
  case TOURNAMENT:{
    // get prediction results
    int lhist_idx = pc & pc_mask;
    int lhist_val = lhistory_table[lhist_idx];
    int local_prediction = lprediction_table[lhist_val];
    int tournament_local_outcome = (local_prediction == WN || local_prediction == SN) ? NOTTAKEN : TAKEN;
    
    int history = global_history & ghistory_mask;
    int address = pc & ghistory_mask;
    int global_prediction = gprediction_table[history ^ address];
    int tournament_global_outcome = (global_prediction >= 2) ? TAKEN : NOTTAKEN;
    // update choice counter
    int p1_correct = tournament_local_outcome == outcome ? 0 : 1;
    int p2_correct = tournament_global_outcome == outcome ? 0 : 1;
    int diff = p1_correct - p2_correct;
    choice_prediction[history] += diff;
    choice_prediction[history] = choice_prediction[history] > 3 ? 3 : choice_prediction[history];
    choice_prediction[history] = choice_prediction[history] < 0 ? 0 : choice_prediction[history];
    //printf("%d", choice_prediction[history]);
    // update table states
    int gtable_idx = (global_history & ghistory_mask) ^ (pc & ghistory_mask);
    if (outcome == NOTTAKEN){
      // update local
      if (local_prediction>0)
        lprediction_table[lhist_val]--;
      //printf("%d , %d\n",lhistory_table[lhist_idx], (int)(lhist_val << 1));
      lhistory_table[lhist_idx] = (lhist_val << 1) & 0b11;
      // update global
      if (gprediction_table[gtable_idx]>0)
        gprediction_table[gtable_idx]--;
      global_history = global_history << 1;
    }
    else{
      // update local
      //printf("%d , %d\n",lhistory_table[lhist_idx], (int)((lhist_val << 1)+1));
      lhistory_table[lhist_idx] = ((lhist_val << 1)+1) & 0b11;
      if (local_prediction<3){
        lprediction_table[lhist_val]++;
      }
      // update global
      if (gprediction_table[gtable_idx]<3){
        gprediction_table[gtable_idx]++;
      }
      global_history = (global_history << 1) + 1;
    }
    return;
  }
  case CUSTOM:{
    int index_mask = (1<<lhistoryBits) -1;
    int history_table_index = pc & index_mask;
    int pattern_index = history_table[history_table_index];
    int local_prediction = pattern_table[pattern_index];
    history_table[history_table_index] = history_table[history_table_index] >> 1;
    int tournament_local_outcome = (local_prediction == WN || local_prediction == SN) ? NOTTAKEN : TAKEN;
    
  
    int history = global_history & ghistory_mask;
    int address = pc & ghistory_mask;
    int global_prediction = gprediction_table[history ^ address];
    int tournament_global_outcome = (global_prediction >= 2) ? TAKEN : NOTTAKEN;
    // update choice counter
    int p1_correct = tournament_local_outcome == outcome ? 0 : 1;
    int p2_correct = tournament_global_outcome == outcome ? 0 : 1;
    int diff = p1_correct - p2_correct;
    choice_prediction[history] += diff;
    choice_prediction[history] = choice_prediction[history] > 3 ? 3 : choice_prediction[history];
    choice_prediction[history] = choice_prediction[history] < 0 ? 0 : choice_prediction[history];

    int gtable_idx = (global_history & ghistory_mask) ^ (pc & ghistory_mask);
    if (outcome == TAKEN){
      if(pattern_table[pattern_index] < 3){
        pattern_table[pattern_index] = pattern_table[pattern_index]+1;
      }
      history_table[history_table_index] |= (1U << (patternBits - 1));
      // update global
      if (gprediction_table[gtable_idx]<3){
        gprediction_table[gtable_idx]++;
      }
      global_history = (global_history << 1) + 1;
    }else{
      if(pattern_table[pattern_index] > 0){
        pattern_table[pattern_index] = pattern_table[pattern_index]-1;
      }
      history_table[history_table_index] &= ~(1U << (patternBits - 1));
      // update global
      if (gprediction_table[gtable_idx]>0)
        gprediction_table[gtable_idx]--;
      global_history = global_history << 1;
    }
  }
  case 4:{

  }
  default:
    break;
  }
}
