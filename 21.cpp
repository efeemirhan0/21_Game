#include <iostream>
#include <string>
#include <vector>
#include <algorithm> 
#include <random>    
#include <thread>      
#include <chrono>
#include <limits>      


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



int getCardValue(const std::string& rank) {
    if (rank == "Vale" || rank == "Kiz" || rank == "Papaz") {
        return 10;
    }
    if (rank == "As") {
        return 11;
    }
    return std::stoi(rank);
}

void createDeck(std::vector<Card>& deck) {
    deck.clear();
    std::string suits[] = {"Kupa", "Maca", "Karo", "Sinek"};
    std::string ranks[] = {"As", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Vale", "Kiz", "Papaz"};
    for (const std::string& s : suits) {
        for (const std::string& r : ranks) {
            deck.push_back({s, r, getCardValue(r)});
        }
    }
}

void shuffleDeck(std::vector<Card>& deck) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    std::shuffle(deck.begin(), deck.end(), g);
}

int calculateHandTotal(const std::vector<Card>& hand) {
    int total = 0;
    int aceCount = 0;
    for (const Card& card : hand) {
        total += card.value;
        if (card.rank == "As") {
            aceCount++;
        }
    }
    while (total > 21 && aceCount > 0) {
        total -= 10;
        aceCount--;
    }
    return total;
}


void printHand(const std::string& name, const std::vector<Card>& hand, bool isDealerHidden = false) {
    std::cout << name << "'in eli: ";
    if (isDealerHidden) {
        std::cout << "[GIZLI KART] ";
        std::cout << hand[1].rank << " " << hand[1].suit << std::endl;
    } else {
        for (const Card& card : hand) {
            std::cout << card.rank << " " << card.suit << " | ";
        }
        std::cout << "Toplam: " << calculateHandTotal(hand) << std::endl;
    }
}

void checkDeck(std::vector<Card>& deck) {
    if (deck.size() < 20) { 
        std::cout << "\n--- Desteniz azaldi! Yeni deste olusturuluyor ve karistiriliyor... ---\n" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        createDeck(deck);
        shuffleDeck(deck);
    }
}

Card dealCard(std::vector<Card>& deck) {
    checkDeck(deck);
    Card drawnCard = deck.back();
    deck.pop_back();
    return drawnCard;
}


void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}



int main() {
    std::vector<Card> deck;
    createDeck(deck);
    shuffleDeck(deck);

   
    std::vector<Player> players;
    int numPlayers = 0;
    while (numPlayers < 1 || numPlayers > 4) {
        std::cout << "Kac oyuncu oynayacak? (1-4): ";
        std::cin >> numPlayers;
        if (std::cin.fail() || numPlayers < 1 || numPlayers > 4) {
            std::cout << "Lutfen 1-4 arasi bir sayi girin." << std::endl;
            clearInputBuffer();
            numPlayers = 0;
        }
    }

    for (int i = 0; i < numPlayers; ++i) {
        std::string name;
        std::cout << (i + 1) << ". Oyuncunun adi: ";
        std::cin >> name;
        players.push_back({name, {}, 100, 0, PLAYING}); 
    }

   
    bool gameIsRunning = true;
    while (gameIsRunning) {
        
        std::cout << "\n--- YENI TUR ---" << std::endl;
        std::vector<Card> dealerHand;
        int activePlayersThisRound = 0;

        
        for (auto& player : players) {
            if (player.status == QUIT) continue; 

            if (player.money <= 0) {
                std::cout << player.name << " parasiz kaldi ve oyundan ayrildi." << std::endl;
                player.status = QUIT;
                continue;
            }

            
            player.hand.clear();
            player.status = PLAYING;
            player.currentBet = 0;

            std::cout << "--------------------" << std::endl;
            std::cout << player.name << " (Bakiye: " << player.money << "$)" << std::endl;
            
            while (true) {
                std::cout << "Bahis gir (Min 1, Max " << player.money << "): ";
                std::cin >> player.currentBet;
                if (std::cin.fail()) {
                    std::cout << "Lutfen sayi girin." << std::endl;
                    clearInputBuffer();
                } else if (player.currentBet > player.money) {
                    std::cout << "Yetersiz bakiye." << std::endl;
                } else if (player.currentBet <= 0) {
                    std::cout << "Gecersiz bahis. (Min 1)" << std::endl;
                } else {
                    break; 
                }
            }
            activePlayersThisRound++;
        }

        if (activePlayersThisRound == 0) {
            std::cout << "Oynayan yok. Oyun bitti." << std::endl;
            gameIsRunning = false;
            continue; 
        }

        
        for (auto& player : players) {
            if (player.status != QUIT) player.hand.push_back(dealCard(deck));
        }
        dealerHand.push_back(dealCard(deck));
        
        for (auto& player : players) {
            if (player.status != QUIT) player.hand.push_back(dealCard(deck));
        }
        dealerHand.push_back(dealCard(deck));

        
        bool dealerHasBJ = (calculateHandTotal(dealerHand) == 21);
        printHand("Kurpiyer", dealerHand, true);

        for (auto& player : players) {
            if (player.status == PLAYING) {
                printHand(player.name, player.hand);
                if (calculateHandTotal(player.hand) == 21) {
                    if (dealerHasBJ) {
                        std::cout << player.name << ": Berabere. Ikisi de Blackjack." << std::endl;
                        player.status = STANDING; 
                    } else {
                        std::cout << player.name << ": BLACKJACK. 1.5 kat odeme alir." << std::endl;
                        player.status = BLACKJACK; 
                    }
                } else if (dealerHasBJ) {
                    std::cout << player.name << ": Kaybettin. Kurpiyerin Blackjack'i var." << std::endl;
                    player.status = BUSTED; 
                }
            }
        }
        
        
        if (!dealerHasBJ) { 
            for (auto& player : players) {
                if (player.status != PLAYING) continue; 

                std::cout << "\n--- " << player.name << "'in turu ---" << std::endl;
                
                while (player.status == PLAYING) {
                    char choice = ' ';
                    while (choice != '1' && choice != '0') {
                        std::cout << player.name << ", Kart mi (1), Durmak mi (0)? ";
                        std::cin >> choice;
                    }

                    if (choice == '1') {
                        player.hand.push_back(dealCard(deck));
                        printHand(player.name, player.hand);
                        if (calculateHandTotal(player.hand) > 21) {
                            std::cout << player.name << " Batti! " << std::endl;
                            player.status = BUSTED;
                        }
                    } else if (choice == '0') {
                        player.status = STANDING;
                    }
                }
            }
        }

        
        bool dealerMustPlay = false;
        for (const auto& player : players) {
            if (player.status == STANDING) { 
                dealerMustPlay = true;
                break;
            }
        }

        bool dealerBusted = false;
        if (dealerMustPlay) {
            std::cout << "\n--- Kurpiyerin Turu ---" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            printHand("Kurpiyer", dealerHand, false); 

            while (calculateHandTotal(dealerHand) < 17) {
                std::cout << "Kurpiyer kart cekiyor..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                dealerHand.push_back(dealCard(deck));
                printHand("Kurpiyer", dealerHand, false);
            }
            
            if (calculateHandTotal(dealerHand) > 21) {
                std::cout << "Kurpiyer Batti." << std::endl;
                dealerBusted = true;
            }
        } else {
             printHand("Kurpiyer", dealerHand, false); 
        }

        
        std::cout << "\n--- SONUCLAR ---" << std::endl;
        int dealerTotal = calculateHandTotal(dealerHand);
        std::cout << "Kurpiyer Toplami: " << dealerTotal << std::endl;

        for (auto& player : players) {
            if (player.status == QUIT) continue;

            int playerTotal = calculateHandTotal(player.hand);
            std::cout << player.name << "'in Toplami: " << playerTotal;

            switch (player.status) {
                case BLACKJACK:
                    player.money += (player.currentBet * 3) / 2;
                    std::cout << " (Blackjack - Bakiye: " << player.money << "$)" << std::endl;
                    break;
                case BUSTED:
                    player.money -= player.currentBet;
                    std::cout << " (Batti - Bakiye: " << player.money << "$)" << std::endl;
                    break;
                case STANDING:
                    if (dealerBusted || playerTotal > dealerTotal) {
                        player.money += player.currentBet;
                        std::cout << " (Kazandi - Bakiye: " << player.money << "$)" << std::endl;
                    } else if (playerTotal < dealerTotal) {
                        player.money -= player.currentBet;
                        std::cout << " (Kaybetti - Bakiye: " << player.money << "$)" << std::endl;
                    } else {
                        std::cout << " (Berabere - Bakiye: " << player.money << "$)" << std::endl;
                    }
                    break;
                default:
                    
                    break;
            }
        }
        
        
        for (auto& player : players) {
            if (player.status == QUIT) continue;
            if (player.money <= 0) {
                 std::cout << player.name << " parasi bitti ve oyundan atildi." << std::endl;
                 player.status = QUIT;
                 continue;
            }
            
            char choice = ' ';
            while (choice != 'e' && choice != 'h') {
                 std::cout << player.name << ", devam etmek istiyor musun? (e/h): ";
                 std::cin >> choice;
            }
            if (choice == 'h') {
                player.status = QUIT;
                std::cout << player.name << " oyundan cikti." << std::endl;
            }
        }
    } 

    std::cout << "\nOynadiginiz icin tesekkurler." << std::endl;
    std::cout << "--- SON BAKIYELER ---" << std::endl;
    for (const auto& player : players) {
        if(player.money > 0) {
             std::cout << player.name << ": " << player.money << "$" << std::endl;
        }
    }

        std::cout << "\n----------------------------------------" << std::endl;
        std::cout << "Cikmak icin 0'a basin." << std::endl;
        std::cout << "Seciminiz: ";

        char choice = ' ';
        std::cin >> choice;

        if (choice == '0') {
            gameIsRunning = false; 
        }

    } 