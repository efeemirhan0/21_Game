#include <iostream>
#include <string>
#include <vector>
#include <algorithm> 
#include <random>    
#include <thread>      
#include <chrono>
#include <limits>      

// Windows için gerekli kütüphane (UTF-8 ayarı için)
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// --- Structs and Enums ---

struct Card {
    string suit; 
    string rank; 
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
    string name;
    vector<Card> hand;
    int money;
    int currentBet;
    PlayerStatus status;
};

// --- HELPER FUNCTIONS ---

int getCardValue(const string& rank) {
    if (rank == "Jack" || rank == "Queen" || rank == "King") return 10;
    if (rank == "Ace") return 11;
    try {
        return stoi(rank);
    } catch (...) {
        return 0;
    }
}

void createDeck(vector<Card>& deck) {
    deck.clear();
    string suits[] = {"Hearts", "Spades", "Diamonds", "Clubs"};
    string ranks[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    
    for (const string& s : suits) {
        for (const string& r : ranks) {
            deck.push_back({s, r, getCardValue(r)});
        }
    }
}

void shuffleDeck(vector<Card>& deck) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937 g(seed);
    shuffle(deck.begin(), deck.end(), g);
}

int calculateHandTotal(const vector<Card>& hand) {
    int total = 0;
    int aceCount = 0;
    for (const Card& card : hand) {
        total += card.value;
        if (card.rank == "Ace") aceCount++;
    }
    while (total > 21 && aceCount > 0) {
        total -= 10;
        aceCount--;
    }
    return total;
}

void checkDeck(vector<Card>& deck) {
    if (deck.size() < 20) { 
        cout << "\n--- Deck is running low! Shuffling... ---\n" << endl;
        this_thread::sleep_for(chrono::milliseconds(1500));
        createDeck(deck);
        shuffleDeck(deck);
    }
}

Card dealCard(vector<Card>& deck) {
    checkDeck(deck);
    Card drawnCard = deck.back();
    deck.pop_back();
    return drawnCard;
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// --- VISUALIZATION (SYMBOLS) ---

// BURAYI GÜNCELLEDİK: ARTIK SEMBOL DÖNDÜRÜYOR
string getSuitSymbol(const string& suit) {
    if (suit == "Hearts")   return "♥"; // Kupa
    if (suit == "Spades")   return "♠"; // Maça
    if (suit == "Diamonds") return "♦"; // Karo
    if (suit == "Clubs")    return "♣"; // Sinek
    return "?";
}

string getRankSymbol(const string& rank) {
    if (rank == "Ace") return "A";
    if (rank == "Jack") return "J";
    if (rank == "Queen") return "Q";
    if (rank == "King") return "K";
    return rank; 
}

void printHandVisual(const string& name, const vector<Card>& hand, bool isDealerHidden = false) {
    cout << "\n" << name << "'s Hand:" << endl;

    if (hand.empty()) return;

    int height = 5; 
    
    for (int row = 0; row < height; ++row) {
        for (int i = 0; i < hand.size(); ++i) {
            
            if (isDealerHidden && i == 0) {
                if (row == 0)      cout << " .---.  ";
                else if (row == 4) cout << " '---'  ";
                else               cout << " |###|  "; 
                continue;
            }

            const Card& card = hand[i];
            string r = getRankSymbol(card.rank);
            string s = getSuitSymbol(card.suit); // Sembolü al

            string rankStr = (r == "10") ? r : (r + " ");

            switch (row) {
                case 0: cout << " .---.  "; break;
                case 1: cout << " |" << rankStr << " |  "; break; // Sol üst sayı
                case 2: cout << " | " << s << " |  "; break;       // ORTA SEMBOL
                case 3: cout << " | " << rankStr << "|  "; break; // Sağ alt sayı
                case 4: cout << " '---'  "; break;
            }
        }
        cout << endl; 
    }
    
    if (!isDealerHidden) {
        cout << "Total: " << calculateHandTotal(hand) << "\n" << endl;
    } else {
         cout << "Total: ?\n" << endl;
    }
}

// --- MAIN PROGRAM ---

int main() {
    // *** BU KISIM ÇOK ÖNEMLİ ***
    // Windows terminalini UTF-8 moduna zorlar.
    // Böylece ♥ ♦ ♣ ♠ karakterleri "bozuk" görünmez.
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    bool fullProgramRunning = true;

    while (fullProgramRunning) {
        
        cout << "\n========================================" << endl;
        cout << "        WELCOME TO BLACKJACK            " << endl;
        cout << "========================================" << endl;

        vector<Card> deck;
        createDeck(deck);
        shuffleDeck(deck);

        vector<Player> players;
        int numPlayers = 0;
        
        while (numPlayers < 1 || numPlayers > 4) {
            cout << "How many players? (1-4): ";
            cin >> numPlayers;
            if (cin.fail() || numPlayers < 1 || numPlayers > 4) {
                cout << "Please enter a number between 1-4." << endl;
                clearInputBuffer();
                numPlayers = 0;
            }
        }

        for (int i = 0; i < numPlayers; ++i) {
            string name;
            cout << (i + 1) << ". Player Name: ";
            cin >> name;
            players.push_back({name, {}, 100, 0, PLAYING}); 
        }

        bool gameIsRunning = true;
        while (gameIsRunning) {
            
            cout << "\n--- NEW ROUND ---" << endl;
            vector<Card> dealerHand;
            int activePlayersThisRound = 0;

            // 1. Betting
            for (auto& player : players) {
                if (player.status == QUIT) continue; 

                if (player.money <= 0) {
                    cout << player.name << " is broke and left." << endl;
                    player.status = QUIT;
                    continue;
                }

                player.hand.clear();
                player.status = PLAYING;
                player.currentBet = 0;

                cout << "--------------------" << endl;
                cout << player.name << " (Balance: $" << player.money << ")" << endl;
                
                while (true) {
                    cout << "Enter Bet (Min 1, Max " << player.money << "): ";
                    cin >> player.currentBet;
                    if (cin.fail()) {
                        cout << "Invalid input." << endl;
                        clearInputBuffer();
                    } else if (player.currentBet > player.money) {
                        cout << "Insufficient funds." << endl;
                    } else if (player.currentBet <= 0) {
                        cout << "Invalid bet." << endl;
                    } else {
                        break; 
                    }
                }
                activePlayersThisRound++;
            }

            if (activePlayersThisRound == 0) {
                gameIsRunning = false;
                continue; 
            }

            // 2. Deal Cards
            for (auto& player : players) {
                if (player.status != QUIT) player.hand.push_back(dealCard(deck));
            }
            dealerHand.push_back(dealCard(deck));
            
            for (auto& player : players) {
                if (player.status != QUIT) player.hand.push_back(dealCard(deck));
            }
            dealerHand.push_back(dealCard(deck));

            bool dealerHasBJ = (calculateHandTotal(dealerHand) == 21);
            
            // Draw Dealer
            printHandVisual("Dealer", dealerHand, true);

            // 3. Player Checks
            for (auto& player : players) {
                if (player.status == PLAYING) {
                    printHandVisual(player.name, player.hand);
                    
                    if (calculateHandTotal(player.hand) == 21) {
                        if (dealerHasBJ) {
                            cout << player.name << ": Push (Tie)." << endl;
                            player.status = STANDING; 
                        } else {
                            cout << player.name << ": BLACKJACK! Pays 3:2." << endl;
                            player.status = BLACKJACK; 
                        }
                    } else if (dealerHasBJ) {
                        cout << player.name << ": Lost against Dealer BJ." << endl;
                        player.status = BUSTED; 
                    }
                }
            }
            
            // 4. Turns
            if (!dealerHasBJ) { 
                for (auto& player : players) {
                    if (player.status != PLAYING) continue; 

                    cout << "\n>>> " << player.name << "'s Turn <<<" << endl;
                    
                    while (player.status == PLAYING) {
                        char choice = ' ';
                        while (choice != '1' && choice != '0') {
                            cout << "Hit (1) or Stand (0)? ";
                            cin >> choice;
                        }

                        if (choice == '1') {
                            player.hand.push_back(dealCard(deck));
                            printHandVisual(player.name, player.hand);
                            
                            if (calculateHandTotal(player.hand) > 21) {
                                cout << player.name << " Busted!" << endl;
                                player.status = BUSTED;
                            }
                        } else if (choice == '0') {
                            player.status = STANDING;
                        }
                    }
                }
            }

            // 5. Dealer Turn
            bool dealerMustPlay = false;
            for (const auto& player : players) {
                if (player.status == STANDING) { 
                    dealerMustPlay = true;
                    break;
                }
            }

            bool dealerBusted = false;
            if (dealerMustPlay) {
                cout << "\n>>> Dealer's Turn <<<" << endl;
                this_thread::sleep_for(chrono::milliseconds(1000));
                printHandVisual("Dealer", dealerHand, false); 

                while (calculateHandTotal(dealerHand) < 17) {
                    cout << "Dealer hits..." << endl;
                    this_thread::sleep_for(chrono::milliseconds(1500));
                    dealerHand.push_back(dealCard(deck));
                    printHandVisual("Dealer", dealerHand, false);
                }
                
                if (calculateHandTotal(dealerHand) > 21) {
                    cout << "Dealer Busted." << endl;
                    dealerBusted = true;
                }
            } else {
                 printHandVisual("Dealer", dealerHand, false); 
            }

            // 6. Results
            cout << "\n--- RESULTS ---" << endl;
            int dealerTotal = calculateHandTotal(dealerHand);
            cout << "Dealer Total: " << dealerTotal << endl;

            for (auto& player : players) {
                if (player.status == QUIT) continue;

                int playerTotal = calculateHandTotal(player.hand);
                cout << player.name << " Total: " << playerTotal;

                switch (player.status) {
                    case BLACKJACK:
                        player.money += (player.currentBet * 3) / 2;
                        cout << " (WON BJ - Balance: $" << player.money << ")" << endl;
                        break;
                    case BUSTED:
                        player.money -= player.currentBet;
                        cout << " (LOST - Balance: $" << player.money << ")" << endl;
                        break;
                    case STANDING:
                        if (dealerBusted || playerTotal > dealerTotal) {
                            player.money += player.currentBet;
                            cout << " (WON - Balance: $" << player.money << ")" << endl;
                        } else if (playerTotal < dealerTotal) {
                            player.money -= player.currentBet;
                            cout << " (LOST - Balance: $" << player.money << ")" << endl;
                        } else {
                            cout << " (PUSH - Balance: $" << player.money << ")" << endl;
                        }
                        break;
                    default: break;
                }
            }
            
            // 7. Continue?
            bool anyoneLeft = false;
            for (auto& player : players) {
                if (player.status == QUIT) continue;
                if (player.money <= 0) {
                     cout << player.name << " is broke." << endl;
                     player.status = QUIT;
                     continue;
                }
                
                anyoneLeft = true;
                char choice = ' ';
                while (choice != 'y' && choice != 'n') {
                     cout << player.name << ", keep playing? (y/n): ";
                     cin >> choice;
                }
                if (choice == 'n') {
                    player.status = QUIT;
                    cout << player.name << " left." << endl;
                }
            }

            if (!anyoneLeft) {
                gameIsRunning = false;
            }

        } 

        cout << "\nGame Over." << endl;
        
        char restartChoice = ' ';
        while (restartChoice != 'y' && restartChoice != 'n') {
            cout << "Restart Program? (y/n): ";
            cin >> restartChoice;
        }

        if (restartChoice == 'n') {
            fullProgramRunning = false; 
        } else {
            clearInputBuffer();
        }

    } 

    return 0;
}