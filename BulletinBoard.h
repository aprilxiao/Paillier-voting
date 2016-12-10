#include <cstdlib>
#include <utility>
#include <ctime>
#include <iostream>
#include <cmath>

#include <gmpxx.h> // for using GMP integers and integer operations.

class BulletinBoard {
  mpz_t ** votes; // a 2-d array of mpz_t (GMP integers) containing the encrypted votes of all elegible voters.
  int numVotes; // an integer storing the number of voters that have voted so far. It is used to add new votes to the current position in votes.
  mpz_t * voteSignatures; // an array parallel to the outer array of votes storing hte signature of that vote.
  mpz_t * voteSum; // an array that stores the sum of all of the votes in each column.  It is stored here so that the memory can be freed by BB destructor
  int numCandidates; // stores the number of candidates.  It is assumed that each candidate corresponds to an index in some list of candidates.  It is assumed that the ordering of candidates in this list never changes.
  int numVoters; // stores the number of voters.  The BB does not know which vote is tied to each candidate.  

  // Variables used for the ZKP:  
  mpz_t zkp_n; // the modulus for the encryption is n^2
  mpz_t zkp_g; // the base g
  //  mpz_t zkp_e; // the random e in {0..A-1} where A-1 < n
  unsigned long int zkp_e;
  mpz_t zkp_c; // the ciphertext of the vote currently being checked
  gmp_randstate_t state; // the state of the gmp random number generator
  
public:
  BulletinBoard(int candidates, int voters);
  ~BulletinBoard();
  mpz_t* getVoteSum();
  bool addVote(mpz_t* vote, mpz_t sig);
  void meh( mpz_t & temp, int temp2 ){
	  mpz_set_ui( temp, 1 );
	  for ( unsigned int i = 0; i < numVoters; i++ ){
		  if ( mpz_cmp_si( votes[i][temp2], 0 ) != 0 ){
			mpz_mul( temp, temp, votes[i][temp2] );
		  }
	  }
  }
  
  // ZKP functions
  void beginZKP(mpz_t c, mpz_t N, mpz_t G);
  unsigned long int getZKP_e();
  bool checkU(mpz_t u, mpz_t v, mpz_t w);
  
private:
  void initRandom();
};

// The BulletinBoard Constructor
// The constructor takes in an integer indicating the number of candidates, an integer indicating the number of voters, and 2 mpz_t numbers (N and G) representing the public key of the encryption.  The public key is needed for the ZKP.
BulletinBoard::BulletinBoard(int candidates, int voters) {
  numCandidates = candidates;
  numVoters = voters;

  //initialize voteSignatures
  voteSignatures = new mpz_t[numVoters];  // my c++ is rusty, I think this allocating an array of mpz_t variables on the heap
  for(int i = 0; i < numVoters; i++) {
    mpz_init(voteSignatures[i]);
  }
  
  //initalize votes
  votes = new mpz_t*[numVoters];
  for(int i = 0; i < numVoters; i++) {
    votes[i] = new mpz_t[numCandidates];
  }
  for(int i = 0; i < numVoters; i++) {
    for(int j = 0; j < numCandidates; j++) {
      mpz_init(votes[i][j]);
    }
  }

  numVotes = 0;

  //initialize voteSum
  voteSum = new mpz_t[numCandidates];
  for(int i = 0; i < numCandidates; i++) {
    mpz_init(voteSum[i]);
  }

  mpz_init(zkp_n);
  mpz_init(zkp_g);
  //  mpz_init(zkp_e);
  mpz_init(zkp_c);

  initRandom();
}

// BulletinBoard destructor
// Frees memory used by the mpz_t variables and the dynammically allocated arrays.
BulletinBoard::~BulletinBoard() {
  mpz_clear(zkp_n);
  mpz_clear(zkp_g);
  //  mpz_clear(zkp_e);
  mpz_clear(zkp_c);

  gmp_randclear(state);

  // clear all mpz_t variables then free the array

  //voteSignatures
  for(int i = 0; i < numVoters; i++) {
    mpz_clear(voteSignatures[i]);
  }
  delete[] voteSignatures;

  //votes
  for(int i = 0; i < numVoters; i++) {
    for(int j = 0; j < numCandidates; j++) {
      mpz_clear(votes[i][j]);
    }
    delete[] votes[i];
  }
  delete[] votes;

  //voteSum
  for(int i = 0; i < numCandidates; i++) {
    mpz_clear(voteSum[i]);
  }
  delete[] voteSum;
}

// This function sums up the encrypted votes for all candidates and returns the result in an array of mpz_t
// The result takes the same form as a vote.
// I am pretty sure that you can return a pointer to mpz_t variables.  If it doesn't compile, then have the main pass in a pointer to an array and it is just modified here
mpz_t* BulletinBoard::getVoteSum() {
  for(int i = 0; i < numVoters; i++) {
    for(int j = 0; j < numCandidates; j++) {
      mpz_add(voteSum[j], voteSum[j], votes[i][j]);
    }
  }

  return voteSum;
}

// the function adds the vote vote to the bulletin board of votes.  The bulletin board does not know the identity of the voter, nor does it know the content of the vote.
// this function assumes that it is only called when another part of the program has already verified that the vote is legitimate.
// returns true if the vote was successfully added to the bulletin board.
// returns false otherwise.
bool BulletinBoard::addVote(mpz_t* vote, mpz_t sig) {
  if(numVotes >= numVoters) {
    // everyone that can vote has already voted.
    return false;
  }

  for(int i = 0; i < numCandidates; i++) {
    mpz_set(votes[numVotes][i], vote[i]);
  }

  mpz_set(voteSignatures[numVotes], sig);

  numVotes++;
  return true;
}

////////////////////////////////////////////////////
//  Zero-Knowledge Proof Functions
////////////////////////////////////////////////////

// Procedure for the ZKP
// For each number to be checked:
// 1) call beginZKP(...) passing in the ciphertext of the vote in question.
// 2) call getZKP_e() to get a random number from 0 to n-1
// 3) call checkU(...) passing in the u to be checked, and v and w from the voter.
// 4) repeat steps 2) and 3) t times.  After t times, the probability that the vote has been forged is (n-1)^-t

// initializes the gmp random number generator
void BulletinBoard::initRandom() {
  gmp_randinit_default(state);
  gmp_randseed_ui(state, time(NULL));

  srand(time(NULL));
}

// // returns a random e in {0..n-1}
// unsigned int BulletinBoard::getZKP_e() {
//   unsigned int result = rand() % (zkp_n - 1);
//   zkp_e = result;
//   return result;
// }

// saves the ciphertext c for the current round of ZKP tests.
void BulletinBoard::beginZKP(mpz_t c, mpz_t N, mpz_t G) {
  mpz_set(zkp_n, N);
  mpz_set(zkp_g, G);
  mpz_set(zkp_c, c);
}

/* // sets the value of the mpz_t reference variable e to a random number from 0 to n-1 */
/* void BulletinBoard::getZKP_e(mpz_t &e) { */
/*   mpz_urandomm(zkp_e, state, zkp_n); // sets zkp_e equal to a random number from 0 to zkp_n - 1 */
/*   mpz_set(e, zkp_e); */
/* } */

// sets zkp_e to a random integer betweek 0 and numCandidates-1
unsigned long int BulletinBoard::getZKP_e() {
  zkp_e = rand() % numCandidates;
  return zkp_e;
}

// returns true if u = (g^v)(c^e)(w^n) (mod n^2)
// returns false otherwise
// the getZKP_e function must be called before each call to checkU.
//
// u is the value of u to be compared to (g^v)(c^e)(w^n)
// v is from the voter. v = r-em
// w is from the voter. w = s(x^-e)(g^((r-em)/n))
bool BulletinBoard::checkU(mpz_t u, mpz_t v, mpz_t w) {
  //test if getZKP_e has been called since the last call to this function

  // test if u = (g^v)(c^e)(w^n)
  mpz_t modulus;
  mpz_init(modulus);
  mpz_mul(modulus, zkp_n, zkp_n);
  
  mpz_t gv;
  mpz_init(gv);
  mpz_powm(gv, zkp_g, v, modulus); // g^v

  mpz_t ce;
  mpz_init(ce);
  mpz_powm_ui(ce, zkp_c, zkp_e, modulus); // c^e

  mpz_t wn;
  mpz_init(wn);
  mpz_powm(wn, w, zkp_n, modulus); // w^n

  mpz_t result;
  mpz_init(result);
  mpz_mul(result, gv, ce);
  mpz_mul(result, result, wn);

  mpz_mod( result, result, modulus );

  bool pass = (mpz_cmp(result, u) == 0);

  // free memory used by temporary variables.
  mpz_clear(modulus);
  mpz_clear(gv);
  mpz_clear(ce);
  mpz_clear(wn);
  mpz_clear(result);
  
  return pass;
}
