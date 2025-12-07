#include <iostream>
#include <string>
#include <vector>
#include <algorithm> 
#include <random>    
#include <thread>      
#include <chrono>
#include <limits>      

// --- Necessary Struct and Enum Definitions ---

struct Card {
    std::string suit;
    std::string rank;
    int value;
};

enum PlayerStatus {
    PLAYING,  
    STANDING, 
    BUSTED,   
    BLACKJACK,
    QUIT      
};

struct Player {
    std::string name;
    std::vector<Card> hand;
    int money;
    int currentBet;
    PlayerStatus status;
};

// --- Helper Functions ---

// Converts card rank to numerical value
int getCardValue(const std::string& rank) {
    if (rank == "Jack" || rank == "Queen" || rank == "King") {
        return 10;
    }
    if (rank == "Ace") {
        return 11;
    }
    try {
        return std::stoi(rank);
    } catch (...) {
        return 0; // Safety return in case of error
    }
}

// Creates a standard 52-card deck
void createDeck(std::vector<Card>& deck) {
    deck.clear();
    // English Suit Names
    std::string suits[] = {"Hearts", "Spades", "Diamonds", "Clubs"};
    // English Rank Names
    std::string ranks[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    
    for (const std::string& s : suits) {
        for (const std::string& r : ranks) {
            deck.push_back({s, r, getCardValue(r)});
        }
    }
}

// Shuffles the deck randomly
void shuffleDeck(std::vector<Card>& deck) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    std::shuffle(deck.begin(), deck.end(), g);
}

// Calculates the total value of a hand, handling Aces (1 or 11)
int calculateHandTotal(const std::vector<Card>& hand) {
    int total = 0;
    int aceCount = 0;
    for (const Card& card : hand) {
        total += card.value;
        if (card.rank == "Ace") {
            aceCount++;
        }
    }
    // Adjust Aces if total is over 21
    while (total > 21 && aceCount > 0) {
        total -= 10;
        aceCount--;
    }
    return total;
}

// Prints a player's hand to the console
void printHand(const std::string& name, const std::vector<Card>& hand, bool isDealerHidden = false) {
    std::cout << name << "'s hand: ";
    if (isDealerHidden) {
        std::cout << "[HIDDEN CARD] ";
        std::cout << hand[1].rank << " of " << hand[1].suit << std::endl;
    } else {
        for (const Card& card : hand) {
            std::cout << card.rank << " of " << card.suit << " | ";
        }
        std::cout << "Total: " << calculateHandTotal(hand) << std::endl;
    }
}

// Checks if the deck is running low and recreates it if necessary
void checkDeck(std::vector<Card>& deck) {
    if (deck.size() < 20) { 
        std::cout << "\n--- Deck is running low! Creating and shuffling a new deck... ---\n" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        createDeck(deck);
        shuffleDeck(deck);
    }
}

// Deals a single card from the deck
Card dealCard(std::vector<Card>& deck) {
    checkDeck(deck);
    Card drawnCard = deck.back();
    deck.pop_back();
    return drawnCard;
}

// Clears the input buffer to prevent skipping inputs
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// --- MAIN FUNCTION ---

int main() {
    // This variable ensures the entire program can restart from scratch
    bool fullProgramRunning = true;

    // --- OUTER LOOP (PROGRAM LOOP) ---
    while (fullProgramRunning) {
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "        WELCOME TO BLACKJACK            " << std::endl;
        std::cout << "========================================" << std::endl;

        std::vector<Card> deck;
        createDeck(deck);
        shuffleDeck(deck);

        std::vector<Player> players;
        int numPlayers = 0;
        
        // Get number of players
        while (numPlayers < 1 || numPlayers > 4) {
            std::cout << "How many players will play? (1-4): ";
            std::cin >> numPlayers;
            if (std::cin.fail() || numPlayers < 1 || numPlayers > 4) {
                std::cout << "Please enter a number between 1 and 4." << std::endl;
                clearInputBuffer();
                numPlayers = 0;
            }
        }

        // Get player names
        for (int i = 0; i < numPlayers; ++i) {
            std::string name;
            std::cout << (i + 1) << ". Player's name: ";
            std::cin >> name;
            players.push_back({name, {}, 100, 0, PLAYING}); 
        }

        // --- INNER LOOP (ROUND LOOP) ---
        bool gameIsRunning = true;
        while (gameIsRunning) {
            
            std::cout << "\n--- NEW ROUND ---" << std::endl;
            std::vector<Card> dealerHand;
            int activePlayersThisRound = 0;

            // 1. Betting Phase
            for (auto& player : players) {
                if (player.status == QUIT) continue; 

                if (player.money <= 0) {
                    std::cout << player.name << " ran out of money and left the game." << std::endl;
                    player.status = QUIT;
                    continue;
                }

                player.hand.clear();
                player.status = PLAYING;
                player.currentBet = 0;

                std::cout << "--------------------" << std::endl;
                std::cout << player.name << " (Balance: $" << player.money << ")" << std::endl;
                
                while (true) {
                    std::cout << "Enter bet (Min 1, Max " << player.money << "): ";
                    std::cin >> player.currentBet;
                    if (std::cin.fail()) {
                        std::cout << "Please enter a valid number." << std::endl;
                        clearInputBuffer();
                    } else if (player.currentBet > player.money) {
                        std::cout << "Insufficient funds." << std::endl;
                    } else if (player.currentBet <= 0) {
                        std::cout << "Invalid bet. (Min 1)" << std::endl;
                    } else {
                        break; 
                    }
                }
                activePlayersThisRound++;
            }

            // Check if any active players remain
            if (activePlayersThisRound == 0) {
                std::cout << "No active players left at the table." << std::endl;
                gameIsRunning = false;
                continue; 
            }

            // 2. Dealing Initial Cards
            for (auto& player : players) {
                if (player.status != QUIT) player.hand.push_back(dealCard(deck));
            }
            dealerHand.push_back(dealCard(deck));
            
            for (auto& player : players) {
                if (player.status != QUIT) player.hand.push_back(dealCard(deck));
            }
            dealerHand.push_back(dealCard(deck));

            bool dealerHasBJ = (calculateHandTotal(dealerHand) == 21);
            printHand("Dealer", dealerHand, true);

            // 3. Check for Initial Blackjack
            for (auto& player : players) {
                if (player.status == PLAYING) {
                    printHand(player.name, player.hand);
                    if (calculateHandTotal(player.hand) == 21) {
                        if (dealerHasBJ) {
                            std::cout << player.name << ": Push (Tie). Both have Blackjack." << std::endl;
                            player.status = STANDING; 
                        } else {
                            std::cout << player.name << ": BLACKJACK! Pays 3:2." << std::endl;
                            player.status = BLACKJACK; 
                        }
                    } else if (dealerHasBJ) {
                        std::cout << player.name << ": Lost. Dealer has Blackjack." << std::endl;
                        player.status = BUSTED; 
                    }
                }
            }
            
            // 4. Players' Turns
            if (!dealerHasBJ) { 
                for (auto& player : players) {
                    if (player.status != PLAYING) continue; 

                    std::cout << "\n--- " << player.name << "'s turn ---" << std::endl;
                    
                    while (player.status == PLAYING) {
                        char choice = ' ';
                        while (choice != '1' && choice != '0') {
                            std::cout << player.name << ", Hit (1) or Stand (0)? ";
                            std::cin >> choice;
                        }

                        if (choice == '1') {
                            player.hand.push_back(dealCard(deck));
                            printHand(player.name, player.hand);
                            if (calculateHandTotal(player.hand) > 21) {
                                std::cout << player.name << " Busted!" << std::endl;
                                player.status = BUSTED;
                            }
                        } else if (choice == '0') {
                            player.status = STANDING;
                        }
                    }
                }
            }

            // 5. Dealer's Turn
            bool dealerMustPlay = false;
            for (const auto& player : players) {
                if (player.status == STANDING) { 
                    dealerMustPlay = true;
                    break;
                }
            }

            bool dealerBusted = false;
            if (dealerMustPlay) {
                std::cout << "\n--- Dealer's Turn ---" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                printHand("Dealer", dealerHand, false); 

                while (calculateHandTotal(dealerHand) < 17) {
                    std::cout << "Dealer draws a card..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    dealerHand.push_back(dealCard(deck));
                    printHand("Dealer", dealerHand, false);
                }
                
                if (calculateHandTotal(dealerHand) > 21) {
                    std::cout << "Dealer Busted." << std::endl;
                    dealerBusted = true;
                }
            } else {
                 printHand("Dealer", dealerHand, false); 
            }

            // 6. Calculate Results
            std::cout << "\n--- RESULTS ---" << std::endl;
            int dealerTotal = calculateHandTotal(dealerHand);
            std::cout << "Dealer Total: " << dealerTotal << std::endl;

            for (auto& player : players) {
                if (player.status == QUIT) continue;

                int playerTotal = calculateHandTotal(player.hand);
                std::cout << player.name << "'s Total: " << playerTotal;

                switch (player.status) {
                    case BLACKJACK:
                        player.money += (player.currentBet * 3) / 2;
                        std::cout << " (Blackjack - Balance: $" << player.money << ")" << std::endl;
                        break;
                    case BUSTED:
                        player.money -= player.currentBet;
                        std::cout << " (Busted - Balance: $" << player.money << ")" << std::endl;
                        break;
                    case STANDING:
                        if (dealerBusted || playerTotal > dealerTotal) {
                            player.money += player.currentBet;
                            std::cout << " (Won - Balance: $" << player.money << ")" << std::endl;
                        } else if (playerTotal < dealerTotal) {
                            player.money -= player.currentBet;
                            std::cout << " (Lost - Balance: $" << player.money << ")" << std::endl;
                        } else {
                            std::cout << " (Push/Tie - Balance: $" << player.money << ")" << std::endl;
                        }
                        break;
                    default:
                        break;
                }
            }
            
            // 7. Check Continuation
            bool anyoneLeft = false;
            for (auto& player : players) {
                if (player.status == QUIT) continue;
                if (player.money <= 0) {
                     std::cout << player.name << " ran out of money and was removed from the game." << std::endl;
                     player.status = QUIT;
                     continue;
                }
                
                // Ask remaining players if they want to continue
                anyoneLeft = true;
                char choice = ' ';
                while (choice != 'y' && choice != 'n') {
                     std::cout << player.name << ", do you want to continue? (y/n): ";
                     std::cin >> choice;
                }
                if (choice == 'n') {
                    player.status = QUIT;
                    std::cout << player.name << " left the game." << std::endl;
                }
            }

            // If everyone left, break the inner loop
            if (!anyoneLeft) {
                gameIsRunning = false;
            }

        } // --- INNER LOOP END (gameIsRunning) ---

        // --- GAME OVER REPORT ---
        std::cout << "\n----------------------------------------" << std::endl;
        std::cout << "Game Over." << std::endl;
        std::cout << "--- FINAL BALANCES ---" << std::endl;
        for (const auto& player : players) {
             std::cout << player.name << ": $" << player.money << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;

        // --- RESTART QUESTION ---
        char restartChoice = ' ';
        while (restartChoice != 'y' && restartChoice != 'n') {
            std::cout << "\nDo you want to restart the program from the beginning? (y/n): ";
            std::cin >> restartChoice;
        }

        if (restartChoice == 'n') {
            fullProgramRunning = false; // Terminate the outer loop
        } else {
            std::cout << "\nRestarting program...\n" << std::endl;
            clearInputBuffer(); // Clear input buffer for new session
        }

    } // --- OUTER LOOP END (fullProgramRunning) ---

    std::cout << "See you next time!" << std::endl;
    return 0;
}