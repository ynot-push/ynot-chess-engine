#include <iostream>
#include "square.h"

int notationToIndex(const std::string& notation){
  int index;  
  if(notation.length() != 2) {
    index = -1; // Invalid notation
    return index;
  }    
  if(notation[0] <'a' || notation[0] > 'h' || notation[1]< '1' || notation[1]> '8')
    {
        index = -1; // Invalid notation
        return index;
    }
    index = (7-(notation[1] - '1')) * 8 + (notation[0] - 'a');
    return index;
}



    std::string indexToNotation(int index){
    std::string notation(2, ' '); // Initialize a string of length 2
    if(index < 0 || index > 63) {
        return ""; // Invalid index
    }
    notation[0]= 'a' + (index%8);
    notation[1]= '1' + (7-(index/8));
    return notation;
    }

