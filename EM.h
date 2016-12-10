#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <functional>

using namespace std;
typedef long long int ll;

class EM{
public:
	//constructor destructor
	EM(vector<string> Voters);//takes in vector of strings idFirstLast and populates the map
	~EM();

	//public functions
	ll getN() const {return n;}//actually voter only needs phin
	ll getPhin() const {return phin;}
	ll random_coprime(ll phi);//calculate e using phin
	ll blinding(ll m,ll e);//voter blinds the message
	bool voterCheck(ll blind_msg, string idFirstLast, ll e, ll& blinded_signature, mpz_t PubKey[3]); //send blind_msg, idFirstLast, and e to EM, EM returns blinded_signature and publicKey
	ll rsa_signature(ll blinded_signature, ll r);//get the final signature
	void get_rand( mpz_t & x ) { mpz_set( x, r ); }

	//Paillier public fucntions
	void encryption(mpz_t & msg_secu, mpz_t m, mpz_t* KPub);
	//msg_secu would be the encrypted msg
	//note that this is for a single 1 or 0, you have to do loop through the vector of vote
	std::vector<int>  doDecryp(mpz_t* VoteSum, int numCandidates);//decrypts the votesum and returns the index of the winner



private:
	ll n,d,phin,p,q;// n=p*q, d is the inverse of e, phin is phi of n
	map<string, bool> validVoter;//<idFirstLast, voted?>
	mpz_t LKey[6], KPub[3],KPri[3];
	mpz_t r;

	//blind signature private functions
	ll gcd(ll a, ll b) {return b == 0 ? a : gcd(b, a % b);}
	ll modInverse(ll a, ll m){
	    a = a%m;
	    for (ll x=1; x<m; x++)
	       if ((a*x) % m == 1)
	          return x;
	}
	ll random_prime(ll num){
	    bool prime[num+1];
	    memset(prime, true, sizeof(prime));
	    for (ll i=2; i*i<=num; i++){
	        if (prime[i] == true){
	            for (ll j=i*2; j<=num; j+=i)
	                prime[j] = false;
	        }
	    }
	    vector<ll> primes;
	    for (int i=2; i<=num; i++)
	       if (prime[i])
	          primes.push_back(i);
	    return primes[rand()%primes.size()];
	}
	ll signing(ll m){
	    ll blinding_signature=1;
	    while(d>0){
	        blinding_signature=(blinding_signature*m)%n;
	        d--;
	    }
	    return blinding_signature;
	}

	//Paillier private functions
	void getRandomPrime(mpz_t numrandom)
	{
	    long seed;
	    gmp_randstate_t staterandom;
	    gmp_randinit_default(staterandom);
	    mpz_t max, min, maxtmp;

	    mpz_init(max);
	    mpz_ui_pow_ui (max, 2, 512);
	    mpz_sub_ui(max, max, 1);
	    mpz_init(min);
	    mpz_ui_pow_ui (min, 2, 511);
	    mpz_init(maxtmp);
	    mpz_sub(maxtmp, max, min); // difference between 2^511 and 2^512 - 1

	    time(&seed);
	    gmp_randseed_ui(staterandom, seed);

	    do
	    {
	        mpz_urandomm(numrandom, staterandom, maxtmp);
	        mpz_add(numrandom, numrandom, min);
	        mpz_nextprime(numrandom, numrandom);
	    }
	    while(mpz_cmp(numrandom, max) >= 0);

	    mpz_clear(max);
	    mpz_clear(min);
	    mpz_clear(maxtmp);
	    gmp_randclear(staterandom);
	}

	// For random r, non prime accepted
	void getRandom(mpz_t numrandom)
	{
	    long seed;
	    gmp_randstate_t staterandom;
	    gmp_randinit_default(staterandom);

	    mpz_t max, min, maxtmp;

	    mpz_init(max);
	    mpz_ui_pow_ui (max, 2, 512);
	    mpz_sub_ui(max, max, 1);
	    mpz_init(min);
	    mpz_ui_pow_ui (min, 2, 511);
	    mpz_init(maxtmp);
	    mpz_sub(maxtmp, max, min); // difference between 2^511 and 2^512 - 1

	    time(&seed);
	    gmp_randseed_ui(staterandom, seed);

	    do
	    {
	        mpz_urandomm(numrandom, staterandom, maxtmp);
	        mpz_add(numrandom, numrandom, min);
	    }
	    while(mpz_cmp(numrandom, max) >= 0);

	    mpz_clear(max);
	    mpz_clear(min);
	    mpz_clear(maxtmp);
	    gmp_randclear(staterandom);
	}

	void getKeyList()
	{
	    mpz_t p1, q1, lambdan, n1, nsquare, g, psubone, qsubone, test;
	    mpz_init(p1);
	    mpz_init(q1);
	    mpz_init(lambdan);
	    mpz_init(n1);
	    mpz_init(nsquare);
	    mpz_init(g);
	    mpz_init(test);
	    mpz_init(psubone);
	    mpz_init(qsubone);

	    // p1 and q1 prime
	    getRandomPrime(p1);
	    getRandomPrime(q1);
	    while (mpz_cmp(p1, q1) == 0)
	    {
	        getRandomPrime(p1);
	    }
	    // n1 = p1 * q1
	    mpz_mul(n1, p1, q1);
	    mpz_mul(nsquare, n1, n1);
	    // λ(n1) = lcm(p1 - 1, q1 - 1)
	    mpz_sub_ui(psubone, p1, 1);
	    mpz_sub_ui(qsubone, q1, 1);
	    mpz_lcm(lambdan, qsubone, psubone);
	    mpz_gcd(test, n1, lambdan);

	    if(mpz_cmp_ui(test, 1) == 0)
	    {
	        /* g, ord n^2 (g) = n * α, α ∈ Zλ(n) and α|λ(n)
	         for time saving, g = 1 + n */
			mpz_add_ui(g, n1, 1);
	        mpz_set(LKey[0], p1);
	        mpz_set(LKey[1], q1);
	        mpz_set(LKey[2], lambdan);
	        mpz_set(LKey[3], n1);
	        mpz_set(LKey[4], g);
	        mpz_set(LKey[5], nsquare);
	    }

	    mpz_clear(p1);
	    mpz_clear(q1);
	    mpz_clear(lambdan);
	    mpz_clear(n1);
	    mpz_clear(nsquare);
	    mpz_clear(g);
	    mpz_clear(test);
	    mpz_clear(psubone);
	    mpz_clear(qsubone);
	}
	// Public Keys: n, g and n^2
	// returns n,g,n^2, kpub[0]=n, [1]=g, [2]=n^2
	void getPubKey (mpz_t TMP[3]){
			mpz_set(TMP[0], LKey[3]);
			mpz_set(TMP[1], LKey[4]);
			mpz_set(TMP[2], LKey[5]);
	}

	void getPriKey (mpz_t KPri[3])
	{
	    // Private Keys: p, q and λ(n)
	    mpz_set(KPri[0], LKey[0]);
	    mpz_set(KPri[1], LKey[1]);
	    mpz_set(KPri[2], LKey[2]);
	}


	/* L(u) = (u - 1) / n, ∀u ∈ Sn = {u | 0 < u < n^2 and u ≡ 1 (mod n)}  */
	void lOfDecrypt(mpz_t resultL, mpz_t u, mpz_t* KPub)
	{
	    mpz_t resultLtmp;
	    mpz_init(resultLtmp);

	    mpz_sub_ui(resultLtmp, u, 1);
	    mpz_divexact(resultL, resultLtmp, KPub[0]);

	    mpz_clear(resultLtmp);
	}

	/* msg_clear = L(c^λ(n) (mod n^2)) / L(g^λ(n) (mod n^2)) (mod n) */
	void decryption(mpz_t & msg_clear, mpz_t msg_secu, mpz_t* KPub, mpz_t* KPri)
	{
	    mpz_t abovetmp, above, belowtmp, below, tmp, left, right;

	    mpz_init(abovetmp);
	    mpz_init(above);
	    mpz_init(belowtmp);
	    mpz_init(below);
	    mpz_init(left);
	    mpz_init(right);
	    mpz_init(tmp);

	    mpz_powm(above, msg_secu, KPri[2], KPub[2]);
	    mpz_powm(below, KPub[1], KPri[2], KPub[2]);
	    lOfDecrypt(abovetmp, above, KPub);
	    //lOfDecrypt(belowtmp, below, KPub); // result equal to λ(n)

	    // a / b mod n = ((a mod n) * (b^(-1) mod n)) mod n
	    mpz_mod(above, abovetmp, KPub[0]);
	    // mpz_invert(below, belowtmp, KPub[0]); simple with line below
	    mpz_invert(below, KPri[2], KPub[0]); // inverse of λ(n) mod n
	    mpz_mul(tmp, above, below);
	    mpz_mod(msg_clear, tmp, KPub[0]);

	    mpz_clear(abovetmp);
	    mpz_clear(above);
	    mpz_clear(belowtmp);
	    mpz_clear(below);
	    mpz_clear(tmp);
	    mpz_clear(left);
	    mpz_clear(right);
	}
};


//constructor
EM::EM(vector<string> Voters){
	mpz_init(r);
	for (int i = 0; i < Voters.size(); i++) {
		validVoter[Voters[i]]=false;
	}//populate the map with voters in the vector and set the default to false

	//choose p,q
	srand(time(NULL));
	p=random_prime(100);
	q=random_prime(100);
	while(p==q){
			q=random_prime(100);
	}

	//calculate n, phin
	n=p*q;
	phin=(p-1)*(q-1);

	for ( unsigned int i = 0; i < 6; i++ )
		mpz_init(LKey[i]);
	for ( unsigned int i = 0; i < 3; i++ ){
		mpz_init( KPub[i] );
		mpz_init( KPri[i] );
	}
    getKeyList();//lkey is populated
    getPubKey(KPub);//KPub is populated
    getPriKey(KPri);//Kpri is populated
}

//destructor
EM::~EM(){
	mpz_clear(r);
	for(int i = 0; i < 6; i++){
			mpz_clear(LKey[i]);
	}
	for(int i = 0; i < 3; i++){
			mpz_clear(KPub[i]);
			mpz_clear(KPri[i]);
	}
}

//calculate e using phin
ll EM::random_coprime(ll phi){
	vector<ll> coprimes;
	for(ll i=2; i<phi; i++){
			if(gcd(i,phi)==1)
			{
					coprimes.push_back(i);
			}
	}
	return coprimes[rand()%coprimes.size()];
}

//voter blinds the message
ll EM::blinding(ll m,ll e){
	ll r=random_coprime(n);
	ll blinding_factor=1;
	while(e>0){
			blinding_factor=(blinding_factor*r)%n;
			e--;
	}
	//cout << "Blinding Factor: " << blinding_factor << endl;
	return (m*blinding_factor)%n;
}

//send blind_msg, idFirstLast, and e to EM, EM returns blinded_signature and publicKey
//note that mpz_t is by default passed by reference
bool EM::voterCheck(ll blind_msg, string idFirstLast, ll e, ll& blinded_signature, mpz_t PubKey[3]){
	map<string,bool>::iterator it;
	it = validVoter.find(idFirstLast);//try to find the voter in map
	if (it==validVoter.end()||it->second==true) //voter is not in the list or voter already voted
		return false;
	else{//valid voter
		//calculate d according to e
		d=modInverse(e,phin);
		//sign it
		blinded_signature=signing(blind_msg);

		//initialize key list
		/*getKeyList();//lkey is populated
		for(int i = 0; i < 3; i++){//initialize private keys
				mpz_init(KPri[i]);
		}*/
		//getPubKey(KPub);//KPub is populated
		//getPriKey(KPri);//Kpri is populated
		for ( unsigned int i = 0; i < 3; i++ ){
			mpz_set(PubKey[i],KPub[i]);
		}

		return true;
	}
}

//get the final signature
ll EM::rsa_signature(ll blinded_signature, ll r){
	ll rsa_sign=(blinded_signature*modInverse(r,n))%n;
 // cout << modInverse(r,n) << endl;
	return rsa_sign;
}

// msg_secu = g^m * r^n (mod n)
void EM::encryption(mpz_t & msg_secu, mpz_t m, mpz_t* KPub){
		mpz_t gpowm, rpown, tmp;
		mpz_init(gpowm);
		mpz_init(rpown);
		mpz_init(tmp);

		getRandom(r);
		mpz_powm(gpowm, KPub[1], m, KPub[2]);
		mpz_powm(rpown, r, KPub[0], KPub[2]);
		mpz_mul(tmp, gpowm, rpown);
		mpz_mod(msg_secu, tmp, KPub[2]);

		mpz_clear(gpowm);
		mpz_clear(rpown);
		mpz_clear(tmp);
}

std::vector<int> EM::doDecryp(mpz_t* VoteSum, int numCandidates){
	//decryption
	mpz_t clearSum[numCandidates];
	for(int i = 0; i < numCandidates; i++){
			mpz_init(clearSum[i]);
	}
	for (int i = 0; i < numCandidates; i++) {
		decryption(clearSum[i], VoteSum[i], KPub, KPri);
	}

	mpz_t max;
	mpz_init(max);
	mpz_set_ui(max,0);
	for (int i = 0; i < numCandidates; i++){
		if(mpz_cmp (clearSum[i], max)>0){//clearSum[i]>max
			mpz_set(max,clearSum[i]);
		}
	}
	std::vector<int> idx;
	for (int i = 0; i < numCandidates; i++){
		if(mpz_cmp (clearSum[i], max)==0){
			 idx.push_back(i);
		 }
	}
	mpz_clear(max);
	for (int i = 0; i < numCandidates; i++){
		mpz_clear(clearSum[i]);
	}
	return idx;
}
