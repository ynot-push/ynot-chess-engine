#include "Rook.h"


Bitboard Rook::RookMask(int square){
    int rank=square/8;
    int file=square%8;
    Bitboard mask=0;
    for(int i=rank+1;i<=6;i++){
        mask |=1ull<<(i*8+file);
    }
    for(int i=1;i<rank;i++){
         mask |=1ull<<(i*8+file);
    }
     for(int i=file+1;i<=6;i++){
         mask |=1ull<<(rank*8+i);
    }
     for(int i=1;i<file;i++){
         mask |=1ull<<(rank*8+i);
    }



    return mask;
}

Bitboard Rook::attacks(int square,Bitboard occupied)
{
    int rank=square/8;
    int file=square%8;
    Bitboard attacks=0;
    int i=1;

    while(true){
        rank+=i;
        if(rank>7){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
    }
    rank=square/8;
    file=square%8;
     while(true){
        rank-=i;
        if(rank<0){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
        
    }
    rank=square/8;
    file=square%8;
     while(true){
        file+=i;
        if(file>7){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
    }
    rank=square/8;
    file=square%8;
     while(true){
        file-=i;
        if( file<0){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
        
    }
    return attacks;
}