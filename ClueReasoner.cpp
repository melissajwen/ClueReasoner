#include "ClueReasoner.h"
using namespace std;

int ClueReasoner::GetPlayerNum(string player)
{
	if (player == case_file)
		return num_players;
	
	for (int i = 0; i < num_players; ++i)
		if (player == players[i])
			return i;
			
	cout<<"Illegal player: "<<player<<endl;
	return -1;
}

int ClueReasoner::GetCardNum(string card)
{
	for (int i = 0; i < num_cards; ++i)
		if (card == cards[i])
			return i;
			
	cout<<"Illegal card: "<<card<<endl;
	return -1;
}

int ClueReasoner::GetPairNum(int player, int card) 
{
	return player * num_cards + card + 1;
}

int ClueReasoner::GetPairNum(string player, string card) 
{
	return GetPairNum(GetPlayerNum(player), GetCardNum(card));
}

int ClueReasoner::Query(string player, string card) 
{
	return solver->TestLiteral(GetPairNum(player,card));
}

string ClueReasoner::QueryString(int return_code)
{
	if (return_code == kFalse)
		return "n";
	else if (return_code == kTrue)
		return "Y";
	else if (return_code == kUnknown)
		return "-";
	else
		return "X";
}

void ClueReasoner::PrintNotepad()
{
	for (int i = 0; i < num_players; ++i)
		cout<<"\t"<<players[i];
	cout<<"\t"<<case_file<<endl;
	
	for (int i = 0; i < num_cards; ++i)
	{
		cout<<cards[i]<<"\t";
		for (int j = 0; j < num_players; ++j)
			cout<<QueryString(Query(players[j], cards[i]))<<"\t";
		
		cout<<QueryString(Query(case_file, cards[i]))<<endl;
	}
}
	
void ClueReasoner::AddInitialClauses()
{
	/* The following code is given as an example to show how to create Clauses and post them to the solver. SatSolver.h uses the following typedefs:
		typedef int Literal;
		typedef std::vector<Literal> Clause;
		
	That is, a Literal (a propositional variable or its negation) is defined as a positive or a negative (meaning that it is in negated form, as in -p or -q) integer, and a Clause is defined as a vector of Literals.
	
	The function GetPairNum(p, c) returns the literal that corresponds to card c being at location p (either a player's hand or the case_file). 
	See ClueReasoner.h, lines 7-31 for a definition of the arrays and variables that you can use in your implementation. 
	*/

	// Each card is in at least one place (including the case file).
	for (int c = 0; c < num_cards; c++)	// Iterate over all cards.
	{
		Clause clause;
		for (int p = 0; p <= num_players; p++)	// Iterate over all players, including the case file (as a possible place for a card).
			clause.push_back(GetPairNum(p, c)); // Add to the clause the literal that states 'p has c'.
		
		solver->AddClause(clause);
	}
	
	Clause clause;
	// If a card is in one place, it cannot be in another place.
	for (int c = 0; c < num_cards; ++c) {
		for (int p1 = 0; p1 <= num_players; ++p1) {
			for (int p2 = p1 + 1; p2 <= num_players; ++p2) {
				if (p1 != p2) { 
					clause.push_back(GetPairNum(p1, c) * -1);
					clause.push_back(GetPairNum(p2, c) * -1);
					solver->AddClause(clause);
					clause.clear();
				}
			}
		}
		clause.clear();
	}
	
	// At least one card of each category is in the case file.
	for (int s = 0; s < num_suspects; ++s) {
		clause.push_back(GetPairNum(case_file, suspects[s]));
	}
	solver->AddClause(clause);
	clause.clear();

	for(int w = 0; w < num_weapons; ++w) {
		clause.push_back(GetPairNum(case_file, weapons[w]));
	}
	solver->AddClause(clause);
	clause.clear();

	for(int r = 0; r < num_rooms; ++r) {
		clause.push_back(GetPairNum(case_file, rooms[r]));
	}
	solver->AddClause(clause);
	clause.clear();

	// No two cards in each category can both be in the case file.
	for (int s1 = 0; s1 < num_suspects; ++s1) {
		for (int s2 = 0; s2 < num_suspects; ++s2) {
			if (s1 != s2) {
				clause.push_back(GetPairNum(case_file, suspects[s1]) * -1);
				clause.push_back(GetPairNum(case_file, suspects[s2]) * -1);
				solver->AddClause(clause);
				clause.clear();
			}
		}
	}
	
	for (int w1 = 0; w1 < num_weapons; ++w1) {
		for(int w2 = 0; w2 < num_weapons; ++w2) {
			if(w1 != w2) {
				clause.push_back(GetPairNum(case_file, weapons[w1]) * -1);
				clause.push_back(GetPairNum(case_file, weapons[w2]) * -1);
				solver->AddClause(clause);
				clause.clear();
			}
		}
	}

	for(int r1 = 0; r1 < num_rooms; ++r1) {
		for(int r2 = 0; r2 < num_rooms; ++r2) {
			if(r1 != r2) {
				clause.push_back(GetPairNum(case_file, rooms[r1]) * -1);
				clause.push_back(GetPairNum(case_file, rooms[r2]) * -1);
				solver->AddClause(clause);
				clause.clear();
			}
		}
	}	
}

void ClueReasoner::Hand(string player, string cards[3])
{
	// GetPlayerNum returns the index of the player in the players array (not the suspects array). Remember that the players array is sorted wrt the order that the players play. Also note that, player_num (not to be confused with num_players) is a private variable of the ClueReasoner class that is initialized when this function is called.
	
	Clause clause;
	for (int i = 0; i < 3; ++i) {
		clause.push_back(GetPairNum(player, cards[i]));
		solver->AddClause(clause);
		clause.clear();
	}
}

void ClueReasoner::Suggest(string suggester, string card1, string card2, string card3, string refuter, string card_shown)
{
	// Note that in the Java implementation, the refuter and the card_shown can be NULL. 
	// In this C++ implementation, NULL is translated to be the empty string "".
	// To check if refuter is NULL or card_shown is NULL, you should use if(refuter == "") or if(card_shown == ""), respectively.

	string cards[3] = {card1, card2, card3};
	int suggesterPlayer = GetPlayerNum(suggester);

	Clause clause;
	if (refuter == "") {
		for (int currentPlayer = 0; currentPlayer < num_players; ++currentPlayer) {
			if (currentPlayer != suggesterPlayer) {
				for (int i = 0; i < 3; ++i) {
					clause.push_back(GetPairNum(currentPlayer, GetCardNum(cards[i])) * -1);
					solver->AddClause(clause);
					clause.clear();
				}
			}
		}
	}
	else {
		int refuterPlayer = GetPlayerNum(refuter);
		int currentPlayer = suggesterPlayer;

		while (currentPlayer != refuterPlayer) {
			if (currentPlayer == num_players - 1) currentPlayer = 0;
			else currentPlayer++;

			if (currentPlayer == refuterPlayer) break;

			for (int i = 0; i < 3; ++i) {
				clause.push_back(GetPairNum(currentPlayer, GetCardNum(cards[i])) * -1);
				solver->AddClause(clause);
				clause.clear();
			}
		}

		if (card_shown == "") {
			clause.push_back(GetPairNum(refuter, card1));
			clause.push_back(GetPairNum(refuter, card2));
			clause.push_back(GetPairNum(refuter, card3));
		}
		else clause.push_back(GetPairNum(refuter, card_shown));

		solver->AddClause(clause);
		clause.clear();
	}		
}

void ClueReasoner::Accuse(string suggester, string card1, string card2, string card3, bool is_correct)
{
	// TO BE IMPLEMENTED AS AN EXERCISE (you don't need to implement this)
}

