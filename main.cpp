#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <gmpxx.h>

#include "EM.h"
#include "BulletinBoard.h"

typedef long long int ll;

// Reads registered voters and eligible canidates
void read( std::map< std::string, std::pair< std::string, bool > > & regVoters,
		   std::vector< std::string > & canidates, std::vector< std::string > & voters,
		   std::ifstream & vFile, std::ifstream & cFile){
	// Initializes all needed variables
	std::string lname, fname, id;
	while ( vFile >> lname ){ // Reads in and stores registered voters
		vFile >> fname;
		vFile >> id;
		voters.push_back( id + " " + lname + ", " + fname );
		std::pair< std::string, bool > p( lname + ", " + fname, false );
		regVoters.insert( std::pair< std::string, std::pair< std::string, bool > >( id, p ) );
	}
	while ( cFile >> lname){ // Reads in and stores eligible canidates
		cFile >> fname;
		canidates.push_back( lname + ", " + fname );
	}
}

int main( int argc, char* argv[] ){
	std::ifstream vFile;
	std::ifstream cFile;
	if ( argc != 3 ){ // Error Checking
		std::cout << "Invalid format\n./test voters.txt canidates.txt\n";
		exit(1);
	}
	else{ //Open files
		vFile.open( argv[1] );
		cFile.open( argv[2] );
		if ( !vFile || !cFile ){ // More Error Checking
			std::cerr << "Can not open the text(s) file " << argv[1] << ", " << argv[2] << std::endl;
			exit(1);
		}
	}

	// Initializing Needed Variables
	// < id, < name, bool > > st. bool = whether or not this id has voted already
	std::map< std::string, std::pair< std::string, bool > > regVoters;
	std::vector< std::string > canidates; // lname, fname
	std::vector< std::string > voters; // id_fname_lname

	// Parsing
	read( regVoters, canidates, voters, vFile, cFile );

	// Voter Interface
	// Initialize the Election Board and Bulletin Board
	EM em( voters );
	BulletinBoard bb( canidates.size(), voters.size() );
	ll e, m, n, phin, blindm, signedm; // Initializes variables that need to be held constant

	n = em.getN();
	phin = em.getPhin();
	e = em.random_coprime( phin );

	int num_of_voters = voters.size();
	while ( num_of_voters > 0 ){ // Stays in loop until either all registered voters vote, or user tells program to stop
		// Initializing some more needed variables
		mpz_t m2, msg_secu, PubKey[3], msg[canidates.size()], msg2[canidates.size()];
		std::string id, lname, fname, yn;

		std::cout << "PLEASE ENTER LAST NAME, FIRST NAME AND ID#: ";
		std::cin >> lname >> fname >> id;

		std::map< std::string, std::pair< std::string, bool > >::iterator itr = regVoters.find( id );
		// Check if voter is registered
		if ( itr != regVoters.end() && (itr->second).first == ( lname + ", " + fname ) ){
			if ( (itr->second).second == true ){ // Checks if voter has already voted
				std::cout << lname << ", " << fname << " HAS ALREADY VOTED\nWOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
				while ( yn != "Y" && yn != "N" ){  // Checks if user input some other character than Y or N
					std::cin >> yn;
					if ( yn != "Y" && yn != "N" )
						std::cout << "\nINVALID INPUT: PLEASE ENTER N FOR NO OR Y FOR YES\nWOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
				}
				if ( yn == "Y" ){
					std::cout << "___________________________________________\n";
					continue;
				}
				break;
			}
		}
		else{ // Voter is not registered
			std::cout << "ERROR: EITHER THE INFORMATION YOU HAVE ENTERED IS WRONG OR YOU ARE NOT A REGISTERED VOTER\n";
			std::cout << "WOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
			while ( yn != "Y" && yn != "N" ){  // Checks if user input some other character than Y or N
				std::cin >> yn;
				if ( yn != "Y" && yn != "N" )
					std::cout << "\nINVALID INPUT: PLEASE ENTER N FOR NO OR Y FOR YES\nWOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
			}
			if ( yn == "Y" ){
				std::cout << "___________________________________________\n";
				continue;
			}
			break;
		}

		mpz_init( m2 );
		mpz_init( msg_secu );
		for ( unsigned int i = 0; i < 3; i++ )
			mpz_init( PubKey[i] );
		for ( unsigned int i = 0; i < canidates.size(); i++ ){
			mpz_init( msg[i] );
			mpz_init( msg2[i] );
		}

		std::cout << "\nPLEASE VOTE BY ENTERING 0 FOR NO OR 1 FOR YES\nNOTE: YOU CAN ONLY VOTE YES ONCE\n\n";

		bool voteCheck = false; // Keeps track of whether or not the voter has already voted yes once before
		for ( unsigned int i = 0; i < canidates.size(); i++ ){
			ll x; // Vote (0 || 1)
			bool check = true; // Helps with users that don't input a 0 || 1
			std::cout << canidates[i] << ": ";
			std::cin >> x;

			if ( x != 0 && x != 1 ){
				check = false; // Indicates that user has failed to input a 0 || 1
				for ( unsigned int z = 0; z < 2; z++ ){ // Gives user two more attempts to fix their input
					std::cout << "WRONG INPUT. PLEASE ENTER 0 FOR NO OR 1 FOR YES\n";
					std::cout << "NOTE: YOU HAVE " << 2 - z << " MORE ATTEMPTS\n\n";
					std::cout << canidates[i] << ": ";
					std::cin >> x;

					if ( x == 0 || x == 1 ){
						check = true; // Indicates that user has given a correct input
						break;
					}
				}
			}

			if ( check == false ){ // If user has failed to provide correct input after 3 attempts, make them start over
				// Ask if Ballot will continue
				std::cout << "YOUR VOTE WAS NOT SUBMITTED\nWOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
				std::cin >> yn;
				break;
			}

			if ( x == 1 ){ // Checks if voter has voted yes for two people
				if ( voteCheck == false ){
					voteCheck = true;
					m = i + 1; //Makes the message = the increment of the canidate the voter is voting for
					mpz_set_ui( m2, m );
				}
				else{ // If voteCheck already equaled 1 and the voter input another 1, then restart
					// Ask if Ballot will continue
					std::cout << "YOUR VOTE WAS NOT SUBMITTED\nNOTE: YOU CANNOT VOTE YES FOR MORE THAN ONE PERSON\n";
					std::cout << "WOULD YOU LIKE TO TRY AGAIN? (Y/N) ";
					std::cin >> yn;
					break;
				}
			}
			mpz_set_ui( msg[i], x ); // Updates users vote array [0|1]/[1|0]
			mpz_set_ui( msg2[i], x );
		}

		//if ( yn == "Y" || yn == "N" ) // Checks if any problems occured in the above code
			//goto Cleanup;
		if ( yn == "N" ){ // Cleaning up
			mpz_clear( m2 );
			mpz_clear( msg_secu );
			for ( unsigned int i = 0; i < 3; i++ )
				mpz_clear( PubKey[i] );
			for ( unsigned int i = 0; i < canidates.size(); i++ )
				mpz_clear( msg[i] );
			break;
		}
		else if ( yn == "Y" ){
			std::cout << "___________________________________________\n";
			continue;
		}

		blindm = em.blinding( m, e ); // Blinds m using the encrypton e

		// Sends blindm to EM and recieves signedm and PubKey in return
		em.voterCheck( blindm, id + " " + lname + ", " + fname, e, signedm, PubKey );
		em.encryption( msg_secu, m2, PubKey );

		mpz_t x[canidates.size()];
		for ( unsigned int i = 0; i < canidates.size(); i++ )
			mpz_init( x[i] );

		for ( unsigned int i = 0; i < canidates.size(); i++ ){ // Encrpyts each element in msg
			em.encryption( msg[i], msg[i], PubKey );
			mpz_t tmp;
			mpz_init( tmp );
			em.get_rand( tmp );
			mpz_set( x[i], tmp );
			mpz_clear( tmp );
		}

		mpz_t zkp_r, zkp_s, zkp_u, zkp_v, zkp_w, signedm2;
		mpz_inits( zkp_r, zkp_s, zkp_u, zkp_v, zkp_w, signedm2, NULL );
		mpz_set_ui( signedm2, signedm );

		gmp_randstate_t state;
		gmp_randinit_default(state);
		gmp_randseed_ui(state, time(NULL));
		ll e2;

		bb.beginZKP( msg_secu, PubKey[0], PubKey[1] );
		bool tmpCheck = true;
		/*for ( unsigned int i = 0; i < canidates.size(); i++ ){
			for ( unsigned int z = 0; z < 20; z++ ){
				mpz_urandomm( zkp_r, state, PubKey[0] );
				mpz_urandomm( zkp_s, state, PubKey[0] );

				mpz_t gpowm, rpown, tmp, tmp2;
				mpz_init(gpowm);
				mpz_init(rpown);
				mpz_init(tmp);
				mpz_init(tmp2);

				mpz_powm(gpowm, PubKey[1], zkp_r, PubKey[2]);
				mpz_powm(rpown, zkp_s, PubKey[0], PubKey[2]);
				mpz_mul(tmp, gpowm, rpown);
				mpz_mod(zkp_u, tmp, PubKey[2]);

				e2 = bb.getZKP_e();

				mpz_mul_ui( tmp, msg2[i], e2 );
				mpz_sub( zkp_v, zkp_r, tmp );
				mpz_t zkp_e2;
				mpz_init( zkp_e2 );
				mpz_set_ui( zkp_e2, e2 );
				mpz_neg( zkp_e2, zkp_e2 );
				mpz_powm( tmp, x[i], zkp_e2, PubKey[2] );
				mpz_mul( zkp_w, tmp, zkp_s );

				mpz_clear( zkp_e2 );
				mpz_clear(gpowm);
				mpz_clear(rpown);
				mpz_clear(tmp);
				mpz_clear(tmp2);

				if ( !(bb.checkU( zkp_u, zkp_v, zkp_w )) ){
					tmpCheck = false;
				}
			}
		}*/

		if ( tmpCheck == true ){
			bb.addVote( msg, signedm2 );
			std::cout << "\nYOUR VOTE HAS BEEN SUBMITTED\nWOULD YOU LIKE TO CONTINUE? (Y/N) ";
			regVoters[id] = std::pair< std::string, bool >( lname + ", " + fname, true );
			std::cin >> yn;
		}
		else{
			std::cout << "\nERROR: ZKP FAILED\nYOUR VOTE HAS NOT BEEN SUBMITTED\nWOULD YOU LIKE TO CONTINUE? (Y/N) ";
			std::cin >> yn;
		}

		//Cleanup:
			if ( yn == "N" ){ // Cleaning up
				gmp_randclear(state);
				mpz_clear( m2 );
				mpz_clear( msg_secu );
				mpz_clear( zkp_r );
				mpz_clear( zkp_s );
				mpz_clear( zkp_u );
				mpz_clear( zkp_v );
				mpz_clear( zkp_w );
				mpz_clear( signedm2 );

				for ( unsigned int i = 0; i < 3; i++ )
					mpz_clear( PubKey[i] );
				for ( unsigned int i = 0; i < canidates.size(); i++ ){
					mpz_clear( msg[i] );
					mpz_clear( msg2[i] );
					mpz_clear( x[i] );
				}
				break;
			}
			else
				std::cout << "___________________________________________\n";
	}

	mpz_t temp[canidates.size()];
	for ( unsigned int i = 0; i< canidates.size(); i++ ){
		mpz_init( temp[i] );
		mpz_t tmp;
		mpz_init( tmp );
		bb.meh( tmp, i );
		mpz_add( temp[i], temp[i], tmp );

	}

	std::cout <<  "\n";
	std::vector<int> temp2 = em.doDecryp( temp, canidates.size() );
	std::cout << "Winner(s):" << "\n";
	for ( unsigned int i = 0; i < temp2.size(); i++ )
		std::cout << canidates[i] << "\n";
//_______________________________________________________________________________________________________________

	//Cleaning up
	vFile.close();
	cFile.close();
	return 0;
}
