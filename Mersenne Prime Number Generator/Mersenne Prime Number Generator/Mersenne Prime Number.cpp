//2.8.26-2.10.26
//This is a tool to automatically list "all" Mersenne primes (primes that are one less than two to the power of a prime exponent ((2^(a prime))-1)) in sequential order, and save them to an outfile if the user chooses to.
//It uses the completed system from the original Prime Number Generator, but performs additional checks to see if each prime is ALSO a Mersenne prime.
//This version ONLY displays and saves the Mersenne primes.

//Preprocessor Libraries
#include <cmath> //For the pow() function
#include <iostream> //For input/output
#include <fstream> //For outfile creation
#include <vector> //For vectors to store Mersenne primes
#include <mutex> //For mutex vector protection
#include <climits> //For the INT_MAX constant
#include <thread> //For running slo-mo tests and mid-generation pausing
#include <atomic> //For mid-generation pausing
#include <chrono> //For running slo-mo tests

using namespace std; //Declare standard namespace

const string INDENT = "\t\t\t"; //Size of indent in wait=3 tabs

atomic<bool> pauseRequested(false); //Universal pause flag
atomic<bool> saveRequested(false); //Universal save flag
atomic<bool> paused(false); //Universal flag to check if currently paused

mutex primesMutex; //Universal mutex flag to prevent modification to either vector while saving to an outfile

void listenForKeyInput(); //Function to detect presses of the enter key while the generator is running and presses of enter or s (to save) while paused
void pauseGeneration(const vector<long long>& MersennePrimes); //Works with listenForKeyInput to stop prime generation when the enter key is pressed, allowing all generated Mersenne primes to be saved as well
void savePrimes(const vector<long long>& MersennePrimes); //Function to save all Mersenne primes generated so far (in the "MersennePrimes" vector) to a text document called "Mersenne Primes.txt"
void wait(); //Pause alternative
void clearCin(); //Clears cin field for wait function or wipe cin memory

int main() {
	char ch;
	cout << "Henry Kredatus's C++ Mersenne Prime Generator\n\n"; //Project title
	cout << "A Mersenne prime is any prime number that is EXACTLY one less than two raised to a prime power.\n";
	cout << "The first seven will appear almost instantly, while the eighth won't appear for several more hours.\n";
	cout << "Any larger Mersenne primes will effectively never appear due to the length of time it would take to reach them.\n\n";
	cout << "Press ENTER to begin generating Mersenne primes. Press ENTER again at anytime to pause generation,\nwhich also gives you the option to save all currently generated primes to a text file: "; //Prompt to press enter to begin

	while (true) {
		cin.get(ch); //Record the entered key
		clearCin(); //Get rid of any trailing characters

		if (ch == '\n') { //Pressing JUST enter begins the prime number generation
			break;
		}

		else {  //If the entered key wasn't s/S or enter, prompts for another key input
			cout << "\nPressed key was not enter (JUST enter). Try again: ";
		}
	}

	cout << endl; //Drop a line before resuming generation

	thread inputThread(listenForKeyInput); //Begin permanently attempting to detect presses of the enter key while the generator runs and presses of s OR enter while paused
	inputThread.detach();

	int potentialPrime = 0; //Runs through every integer starting with zero (up to just below the integer limit for ints)
	long long potentialLargePrime = static_cast<long long>(INT_MAX); //Used to test potential prime numbers starting with the integer limit for ints (starts right where the int checks end)
	int potentialPrimeExponent = 2; //Used to determine if each prime is one less than a power of two (if so, this number, the exponent, must also be prime)
	//potentialPrimeExponent doesn't need to reset every loop because each power of two for each Mersenne prime is going to be bigger than the last

	vector<long long>MersennePrimes; //Create a vector to store every generated Mersenne prime

	while (potentialPrime < INT_MAX) { //Loop through all integers (below the integer limit for ints)

		bool neitherCheck = false; //Start by assuming the integer IS prime or composite
		bool compositeCheck = false; //Start by assuming the integer IS prime specifically

		if (potentialPrime > 1) { //Primes must be at least two by definition (first check)

			//Former attempt to save time, the thought being excluding multiples of two greater than two would be faster as no such numbers can be prime (second check)
			//Didn't work due to factoring in the extra if/else check being slower than running the nested for loop
			//if ((potentialPrime % 2 != 0) || (potentialPrime == 2)) {

			int potentialFactor = 2; //Runs through every possible integer factor for the number whose prime status is being tested starting with two, up to the potentialPrime's square root, or the first factor it hits

			while (potentialFactor * potentialFactor <= potentialPrime) { //Loop through all integers between two and the square root of the number whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

				//Final check
				if (potentialPrime % potentialFactor == 0) { //If the remainder from dividing the potential prime integer by a potential integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that integer immediately)
					compositeCheck = true; //Set composite status to true
					break;
				}

				potentialFactor++; //Move on to the next potential factor (integer) if the previous integer wasn't a factor
			}

			//} //End former "if multiple of two that's greater than two" check

			//else {
			//	compositeCheck = true; //If the number was a multiple of two other than two, that means it's composite
			//}

		} //End "is prime or composite" check

		else {
			neitherCheck = true; //If the tested integer above was zero or one, set the flag for being neither prime nor composite to true
		}

		if (compositeCheck == false && neitherCheck == false) { //If the number wasn't proven to be zero, one, or composite, then it's prime and its Mersenne prime status can be tested as well

			while ((pow(2, potentialPrimeExponent) - 1) < potentialPrime) { //Find the smallest power of two that's at least one higher than potentialPrime
				potentialPrimeExponent++;
			}

			if ((pow(2, potentialPrimeExponent) - 1) == potentialPrime) { //If the potentialPrime is EXACTLY one less than the first power of two that's larger than it, that power's prime status (and by extension, potentialPrime's Mersenne prime status) can then be checked

				int potentialPrimeExponentFactor = 2; //Runs through every possible integer factor for the exponent whose prime status is being tested starting with two, up to the potentialPrimeExponent's square root, or the first factor it hits
				bool exponentCompositeCheck = false; //Start by assuming the exponent IS prime

				while (potentialPrimeExponentFactor * potentialPrimeExponentFactor <= potentialPrimeExponent) { //Loop through all integers between two and the square root of the exponent whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

					if (potentialPrimeExponent % potentialPrimeExponentFactor == 0) { //If the remainder from dividing the potential prime exponent by a potential integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that exponent immediately)
						exponentCompositeCheck = true; //Set composite status to true
						break;
					}

					potentialPrimeExponentFactor++; //Move on to the next potential factor (integer) if the previous integer wasn't a factor
				}

				if (exponentCompositeCheck == false) { //Doesn't need to perform a neither check as the long long loop starts at the integer limit for ints
					lock_guard<mutex> lock(primesMutex); //Lock when saving a Mersenne prime to the vector
					MersennePrimes.push_back(potentialPrime); //If the exponent wasn't proven to be composite and the power of two is one greater than the prime, then the prime is a Mersenne prime and stored in the MersennePrimes vector
					cout << potentialPrime << endl; //If the exponent wasn't proven to be composite and the power of two is one greater than the prime, then the prime is a Mersenne prime and printed in the list
				}

			} //End "is a prime that's one less than a power of two" check

		} //End "is prime" check

		//this_thread::sleep_for(chrono::seconds(1)); //Delay used to put generator in slo-mo during debugging

		if (pauseRequested) { //If the pauseRequested flag is triggered by an enter press, turns the flag off and and runs the pauseGeneration function (enabling saving as well)
			pauseRequested = false;
			paused = true;
			pauseGeneration(MersennePrimes);
		}

		potentialPrime++; //Move on the next integer

	} //End big integer while loop

	//The generator uses two loops-one for "small" integers, and one for any integers at or above the int limit for ints
	//This allows the generator to perform at max speed while running through smaller integers without it being forced to stop (and roll over) when the int integer limit is reached
	//The loop switch and the eighth and "final" prime appearing took roughly three hours of the generator constantly running to occur during my testing
	//Note that while this does let it continue beyond the int integer limit (which is also the eighth Mersenne prime, and where the second loop starts), it will never reach the ninth one due to it being far too large

	long long potentialLargePrimeExponent = potentialPrimeExponent; //Used to determine if the prime is one less than a power of two (if so, this number, the exponent, must also be prime)
	//Picks up at the previous power of two, as it wouldn't make sense to start from the beginning, nor would it make sense to reset it every loop

	while (true) { //Loop through all integers up to the integer limit for long longs (effectively forever), starting right where the previous loop left off

		bool largePrimeCompositeCheck = false; //Start by assuming the long long integer IS prime specifically
		long long potentialLargePrimeFactor = 2; //Runs through every possible integer factor for the number whose prime status is being tested starting with two, up to the potentialLargePrime's square root, or the first factor it hits

		while (potentialLargePrimeFactor <= potentialLargePrime / potentialLargePrimeFactor) { //Loop through all integers between two and the square root of the number whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

			//Check if the long long integer is prime or composite
			if (potentialLargePrime % potentialLargePrimeFactor == 0) { //If the remainder from dividing the potential prime long long integer by a potential long long integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that long long integer immediately)
				largePrimeCompositeCheck = true; //Set composite status to true
				break;
			}

			potentialLargePrimeFactor++; //Move on to the next potential factor (long long integer) if the previous long long integer wasn't a factor
		}

		if (largePrimeCompositeCheck == false) { //Doesn't need to perform a neither check as the long long loop starts one above the integer limit for ints

			while ((pow(2, potentialLargePrimeExponent) - 1) < potentialLargePrime) { //Find the smallest power of two that's at least one higher than potentialPrime
				potentialLargePrimeExponent++;
			}

			if ((pow(2, potentialLargePrimeExponent) - 1) == potentialLargePrime) { //If the potentialPrime is EXACTLY one less than the first power of two that's larger than it, that power's prime status (and by extension, potentialPrime's Mersenne prime status) can then be checked

				long long potentialLargePrimeExponentFactor = 2; //Runs through every possible long long integer factor for the exponent whose prime status is being tested starting with two, up to the potentialLargePrimeExponent's square root, or the first factor it hits
				bool largeExponentCompositeCheck = false; //Start by assuming the exponent IS prime

				while (potentialLargePrimeExponentFactor <= potentialLargePrimeExponent / potentialLargePrimeExponentFactor) { //Loop through all integers between two and the square root of the exponent whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

					if (potentialLargePrimeExponent % potentialLargePrimeExponentFactor == 0) { //If the remainder from dividing the potential prime exponent by a potential integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that exponent immediately)
						largeExponentCompositeCheck = true; //Set composite status to true
						break;
					}

					potentialLargePrimeExponentFactor++; //Move on to the next potential factor (long long integer) if the previous long long integer wasn't a factor
				}

				if (largeExponentCompositeCheck == false) { //Doesn't need to perform a neither check as the long long loop starts one above the integer limit for ints
					lock_guard<mutex> lock(primesMutex); //Lock when saving a Mersenne prime to the vector
					MersennePrimes.push_back(potentialPrime); //If the exponent wasn't proven to be composite and the power of two is one greater than the prime, then the prime is a Mersenne prime and stored in the MersennePrimes vector
					cout << potentialLargePrime << endl; //If the exponent wasn't proven to be composite and the power of two is one greater than the prime, then the prime is a Mersenne prime and printed in the list
				}

			} //End "is a prime that's one less than a power of two" check
		}

		//this_thread::sleep_for(chrono::seconds(1)); //Delay used to put generator in slo-mo during debugging

		if (pauseRequested) { //If the pauseRequested flag is triggered by an enter press, turns the flag off and and runs the pauseGeneration function (enabling saving as well)
			pauseRequested = false;
			paused = true;
			pauseGeneration(MersennePrimes);
		}

		potentialLargePrime++; //Move on the next long long integer

	} //End big long long while loop

	wait();
	return 0;
}








void listenForKeyInput() { //Function to detect presses of the enter key while the generator is running and presses of enter or s (to save) while paused
	while (true) { //Constantly running for one reason or the other
		char ch = cin.get(); //Permanently attempts to detect presses of the enter key while the generator runs and presses of s OR enter while paused

		if (ch == '\n') {
			if (!paused) {
				pauseRequested = true; //Pauses if enter is pressed and generator is currently running
			}

			else if (!saveRequested)
			{
				paused = false; //Resumes if enter is pressed again, currently paused, and NOT currently saving
			}
		}

		if ((ch == 's' || ch == 'S') && paused) { //If currently paused, pressing s sets saveRequested to true and runs the save function
			saveRequested = true;
		}
	}
}








void pauseGeneration(const vector<long long>& MersennePrimes) { //Works with listenForKeyInput to stop Mersenne prime generation when the enter key is pressed, allowing all generated Mersenne primes to be saved as well
	cout << "Mersenne prime generation paused.\n";
	cout << "Press ENTER to resume, or press S then ENTER to save: ";

	while (paused) { //The REAL pause happens here, as this loop (and therefore, function) won't end and allow the generator loop in main to continue until listenForKeyInput detects that enter is pressed again
		if (saveRequested) { //Runs the save function if listenForKeyInput detects a save request
			saveRequested = false;
			savePrimes(MersennePrimes);
		}

		this_thread::sleep_for(chrono::milliseconds(10)); //Helps conserve CPU usage
	}

	cout << endl; //Drop a line before resuming generation
}








void savePrimes(const vector<long long>& MersennePrimes) { //Function to save all Mersenne primes generated so far (in the "MersennePrimes" vector) to a text document called "Mersenne Primes.txt"
	lock_guard<mutex> lock(primesMutex); //Lock during the entire save to file process
	cout << "\nSaving all generated primes to \"Mersenne Primes.txt\"-This might take a bit-Check the source files afterward!\n\n";
	cout << "Now saving...\n\n";

	ofstream outfile("Mersenne Primes.txt"); //Create outfile variable

	if (outfile.is_open()) { //Makes sure outfile was created and opened successfully

		outfile << MersennePrimes.size() << " Mersenne primes have been generated so far.\n"; //Print the current number of generated and saved Mersenne primes

		outfile << "Largest Mersenne prime generated so far: " << MersennePrimes.back() << endl; //Print the largest currently-generated Mersenne prime.

		outfile << "All Mersenne primes generated so far:\n"; //List heading

		for (int i = 0; i < MersennePrimes.size();i++) { //Loop through the entire Mersenne Primes vector
			outfile << MersennePrimes.at(i) << endl; //Print every generated Mersenne prime on its on line
		}

		cout << "Mersenne primes saved successfully-Press the ENTER key to resume generation. "; //Confirm successful save
	}

	else { //Error message if outfile fails to open, which should never happen
		cout << "Cannot open file!\n";
		wait();
		exit(0);
	}
}








void wait() //Pause alternative
{
	clearCin();
	char ch;
	cout << endl << INDENT << "Press the Enter key to continue ... ";
	cin.get(ch);
}








void clearCin()
{
	//The following if-statement checks to see how many characters are in cin's buffer
	//If the buffer has characters in it, the ignore method gets rid of them.
	//If cin is in the fail state, clear puts it back to the ready state.
	//If cin is not already in the fail state, it still doesn't hurt to call the clear function.
	if (cin.rdbuf()->in_avail() > 0) //If the buffer is empty skip clear and ignore
	{
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); //Clear the input buffer
	}
}