#include <iostream>
#include <string>
#include <list>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

double normal_rand()
{
	return static_cast<double>(rand()) / RAND_MAX;
}

unsigned int check_sum(string &str)
{
	unsigned int sum = 0;
	for(string::iterator it=str.begin(); it!=str.end(); ++it)
		sum += pow(2, *it-'A');
	return sum;
}


class Pile
{
private:
	list<char> _letters;
public:
	Pile(int round)
	{
		const int size = round * 2;
		list<char> seq;
		for(char i='A', iend='A'+size; i<iend; ++i)
			seq.push_back(i);

		int m;
		list<char>::iterator it;
		char letter;
		for(int i=size; i>0; --i)
		{
			m = normal_rand() * i;
			it = seq.begin();
			advance(it, m);
			letter = *it;
			seq.erase(it);
			_letters.push_back(letter);
		}
	}

	auto size() const
	{
		return _letters.size();
	}
	char first_letter()
	{
		return _letters.front();
	}
	char second_letter()
	{
		return *(++_letters.begin());
	}
	char take_1()
	{
		char chr = _letters.front();
		_letters.pop_front();
		return chr;
	}
	char take_2()
	{
		auto it = ++_letters.begin();
		char chr = *it;
		_letters.erase(it);
		return chr;
	}
	void put_back_1(const char letter)
	{
		_letters.push_front(letter);
	}
	void put_back_2(const char letter)
	{
		_letters.insert(++_letters.begin(), letter);
	}
	const list<char>& letters()
	{
		return _letters;
	}
};

class Entry    // dictionary entry
{
public:
	string word;
	int score;
};

class Dictionary
{
public:
	Entry *entries;
	int size;

	Dictionary(int size, int round):
		entries(new Entry[size]),size(size)
	{	
		const int 
			pile_size = round * 2,
			max_word_length = round,
			min_word_length = 1,
			word_length_limit_diff = max_word_length - min_word_length + 1;
		unsigned int *check_sums = new unsigned int[size];
		list<char> seq;
		for(char i='A', iend=i+pile_size; i<iend; ++i)
			seq.push_back(i);
		for(int i=0; i<size; )
		{
			int word_length = normal_rand() * word_length_limit_diff + min_word_length;
			string word;
			for(int j=0; j<word_length; ++j)
			{
				int m = normal_rand() * (pile_size - j);
				list<char>::iterator it = seq.begin();
				advance(it, m);
				char letter = *it;
				seq.erase(it);
				word += letter;
			}
			unsigned int checksum = check_sum(word);
			bool exist_anagram = false;
			for(int j=0; j<i; ++j)
			{
				if(check_sums[j] == checksum)
				{
					exist_anagram = true;
					break;
				}
			}
			if(!exist_anagram)
			{
				entries[i].word = word;
				int max_score, min_score, limit_score_diff;
				max_score = word_length * word_length;
				min_score = (word_length - 1) * (word_length - 1) + 1;
				limit_score_diff = max_score - min_score + 1;
				int l = normal_rand() * limit_score_diff + min_score;
				entries[i].score = l;
				check_sums[i] = checksum;
				++i;
			}
			for(string::iterator it=word.begin(); it!=word.end(); ++it)
				seq.push_back(*it);
		}
		delete [] check_sums;
	}
	Dictionary(const Dictionary &original):
		size(original.size),entries(new Entry[original.size])
	{
		for(int i=0; i<size; ++i)
			entries[i] = original.entries[i];
	}
	~Dictionary()
	{
		delete [] entries;
	}
};

class Player
{
private:
	bool _letters[26], *_letters_offset;
public:
	Player()
	{
		for(int i=0; i<26; ++i)
			_letters[i]=false;
		_letters_offset=_letters-'A';
	}
	Player(const Player &value)
	{
		for(int i=0; i<26; ++i)
			_letters[i]=value._letters[i];
		_letters_offset=_letters-'A';
	}
	bool& own(const char letter){
		return _letters_offset[letter];
	}
};

class Score{
public:
	int score_player1, score_player2;
	Score(int score_player1, int score_player2):
		score_player1(score_player1), score_player2(score_player2)
	{}
	int diff()
	{
		return score_player1 - score_player2;
	}
};

class Game
{
public:
	Dictionary dictionary;
	Pile pile;
	Player player1, player2;
	int round;  // 1: player1, 2: player2

private:
	Score _rvo_score;

public:
	Game(int round, int dict_size):
		pile(round),
		dictionary(dict_size, round),
		round(1),
		_rvo_score(123,456)
	{}

	Score& current_score()
	{
		_rvo_score.score_player1 = _rvo_score.score_player2 = 0;
		for(int i=0; i<dictionary.size; ++i)
		{
			const Entry &entry = dictionary.entries[i];
			bool f_p1, f_p2;
			f_p1 = f_p2 = true;
			const string &word = entry.word;
			for(string::const_iterator it=word.begin(); it!=word.end(); ++it)
			{
				char chr = *it;
				if(f_p1)
				{
					if(!player1.own(chr))
					{
						f_p1=false;
						if(!f_p2 || !player2.own(chr))
						{
							goto no_fit;
						}
					}
					else
					{
						f_p2=false;
						continue;
					}
				}
				else if(!f_p2 || !player2.own(chr))
				{
					goto no_fit;
				}
			}

			if(f_p1)
				_rvo_score.score_player1 += entry.score;
			else
				_rvo_score.score_player2 += entry.score;

		no_fit:
			;
		}
		return _rvo_score;
	}

	char player_take(int letter_position)
	{
		char letter;
		if(letter_position == 1)
		{
			letter = pile.take_1();
		}
		else
		{
			letter = pile.take_2();
		}
		if(round == 1)
		{
			player1.own(letter) = true;
			round = 2;
		}
		else
		{
			player2.own(letter) = true;
			round = 1;
		}

		return letter;
	}

	void reverse_player_take(char letter, int position)
	{
		if(position == 1)
		{
			pile.put_back_1(letter);
		}
		else
		{
			pile.put_back_2(letter);
		}
		if(round == 1)
		{
			player2.own(letter) = false;
			round = 2;
		}
		else
		{
			player1.own(letter) = false;
			round = 1;
		}
	}
};


class minimax_value{
public:
	Score score;
	char choice;
	minimax_value(int s_p1, int s_p2):
		score(s_p1, s_p2)
	{}
	minimax_value(Score &score):
		score(score)
	{}
};

minimax_value minimax(Game &game, int alpha, int beta, bool f_max)
{
	Pile &pile = game.pile;
	int &round = game.round;
	if(pile.size() == 0)
	{
		return minimax_value(game.current_score());
	}
	else
	{
		char letter;

		letter = game.player_take(1);
		minimax_value value_first_letter = minimax(game, alpha, beta, !f_max);
		value_first_letter.choice = letter;
		game.reverse_player_take(letter, 1);

		int value_first_letter_score_diff = value_first_letter.score.diff();

		if(f_max)
		{
			if(value_first_letter_score_diff >= beta)
			{
				return value_first_letter;
			}
			alpha = value_first_letter_score_diff;
		}
		else
		{
			if(value_first_letter_score_diff <= alpha)
			{
				return value_first_letter;
			}
			beta = value_first_letter_score_diff;
		}

		if(pile.size() > 1)
		{
			letter = game.player_take(2);
			minimax_value value_second_letter = minimax(game, alpha, beta, !f_max);
			value_second_letter.choice = letter;
			game.reverse_player_take(letter, 2);

			int value_second_letter_score_diff = value_second_letter.score.diff();

			if(f_max)
			{
				if(value_first_letter_score_diff >= value_second_letter_score_diff)
				{
					return value_first_letter;
				}
				else
				{
					return value_second_letter;
				}
			}
			else
			{
				if(value_first_letter_score_diff <= value_second_letter_score_diff)
				{
					return value_first_letter;
				}
				else
				{
					return value_second_letter;
				}
			}
		}
		else
		{
			return value_first_letter;
		}
	}
}

void print(Pile &pile)
{
	const auto &list = pile.letters();
	for(auto it=list.begin(); it!=list.end(); ++it)
		cout << *it << ' ';
	cout << endl;
}

void print(Entry &entry)
{
	cout << entry.word << ' ' << entry.score << endl;
}

void print(Dictionary &dictionary)
{
	for(int i=0; i<dictionary.size; ++i)
		print(dictionary.entries[i]);
}

void print(Player &player)
{
	for(char i='A'; i<='Z'; ++i)
		if(player.own(i))
			cout << i << ' ';
	cout << endl;
}

void display_0()
{
	cout << "========Letter choosing challenge========\n"
		"[Welcome]\n"
		"1: New game\n"
		"2: User manual\n"
		"*: Quit\n"
		"\n> ";
}

void display_1()
{
	cout << "[New game]\n"
		"1: Easy\n"
		"2: Hard\n"
		"3: Custom\n"
		"*: Back\n"
		"\n> ";
}

void display_2()
{
	cout << "[User manual]\n"
		"At the beginning the 2 players see a pile of letters, all different. "
		"In turns they can choose to get the first or the second letter of the"
		" pile. At the last turn, the last player has no choice but getting "
		"the last letter.\nThen, the players score according to the words they"
		" can build with their collected letters. A dictionary gives the "
		"possible words with their associated gains. Letters can be used "
		"several times to form several words.\n\n"
		"The purpose is to get highest positive different between your score "
		"and your opponent's score. For example, '10-2' is a better result "
		"than '15-14', which is better than '20-25'. Computer player is always "
		"choose the best option.\n\n"
		"This puzzle is originated by Wildcat on Codingame.com as a practice "
		"to implement Mini-Max algorithm and alpha-beta pruning.\n\n"
		"by yuholai\n\n";
}

void display_4_3()
{
	cout << "[New game]\n"
		"Input the number of round and the size of dictionary. "
		"Otherwise, go back to the welcome page.\n"
		"\n> ";
}

void display_5()
{
	cout << "[Game start]\n";
}

void display_6_1(Game &game)
{
	cout << "Dictionary: \n";
	print(game.dictionary);
	cout << endl;

	cout << "[Your turn]\n";
	cout << "Pile              : ";
	print(game.pile);

	cout << "Your letters      : ";
	print(game.player1);
	cout << "Opponent's letters: ";
	print(game.player2);
	cout << endl;

	char first_letter;
	first_letter = game.pile.first_letter();
	cout << "1: take " << first_letter;
	if(game.pile.size() > 1)
	{
		char second_letter;
		second_letter = game.pile.second_letter();
		cout << "\t2: take " << second_letter << endl;
	}

	cout << "q: quit\n"
		"\n> ";
}

void display_7(char letter)
{
	cout << "You take " << letter << endl;
}

void display_9(Score &score, minimax_value &best_score)
{
	cout << "[Game end]\n[Score board]\n"
		<< "Your score      : " << score.score_player1 << endl
		<< "Opponent's score: " << score.score_player2 << endl
		<< endl
		<< "Highest score difference: " << best_score.score.diff() << endl
		<< "Your score difference   : " << score.diff() << endl
		<< endl
		<< "1: play again\n"
		"2: new game\n"
		"*: back\n"
		"\n> ";
}

void display_10(char letter)
{
	cout << "Opponent takes " << letter << endl << endl;
}

int main()
{
	srand(time(NULL));

	char chr;
	int diffculty;
	int round, dictionary_size;
	char letter;
	Game *game_original, *game;
	minimax_value best_score(1,1);
	int state;

	state = 0;
	bool f_continue = true;
	do{
		switch(state)
		{
		case 0:
			display_0();
			cin >> chr;
			switch(chr)
			{
			case '1':
				state = 1;
				break;
			case '2':
				state = 2;
				break;
			default:
				state = 3;
			}
			break;
		case 1:
			display_1();
			cin >> chr;
			switch(chr)
			{
			case '1':
			case '2':
			case '3':
				state = 4;
				diffculty = chr - '1';
				break;
			default:
				state = 0;
			}
			break;
		case 2:
			display_2();
			system("PAUSE");
			state = 0;
			break;
		case 3:
			f_continue = false;
			break;
		case 4:
			switch(diffculty)
			{
			case 0:
				round = 2;
				dictionary_size = 4;
				state = 5;
				break;
			case 1:
				round = 4;
				dictionary_size = 16;
				state = 5;
				break;
			case 2:
				display_4_3();
				if(cin >> round >> dictionary_size)
				{
					state = 5;
				}
				else
				{
					state = 0;
				}
			}
			break;
		case 5:
			game_original = new Game(round, dictionary_size);
			game = new Game(*game_original);
			best_score = minimax(*game_original, -99999, 99999, true);
			display_5();
			state = 6;
			break;
		case 6:
			if(game->pile.size() > 0)
			{
				bool f_continue2 = true;
				do{
					display_6_1(*game);
					cin >> chr;
					switch(chr)
					{
					case '1':
						letter = game->pile.letters().front();
						game->player_take(1);
						state = 7;
						f_continue2 = false;
						break;
					case '2':
						if(game->pile.size() > 1)
						{
							letter = *(++game->pile.letters().begin());
							game->player_take(2);
							state = 7;
							f_continue2 = false;
							break;
						}
						break;
					case 'q':
						state = 8;
						f_continue2 = false;
						break;
					}
				}while(f_continue2);
			}
			else
			{
				state = 9;
			}
			break;
		case 7:
			display_7(letter);
			state = 10;
			break;
		case 8:
			delete game;
			delete game_original;
			state = 0;
			break;
		case 9:
			display_9(game->current_score(), best_score);
			cin >> chr;
			delete game;
			switch(chr)
			{
			case '1':
				game = new Game(*game_original);
				state = 6;
				break;
			case '2':
				delete game_original;
				state = 1;
				break;
			default:
				delete game_original;
				state = 0;
			}
			break;
		case 10:
			if(game->pile.size() > 0)
			{
				minimax_value best_choice = minimax(*game, -99999, 99999, false);
				if(best_choice.choice == game->pile.first_letter())
				{
					game->player_take(1);
				}
				else
				{
					game->player_take(2);
				}
				display_10(best_choice.choice);
				state = 6;
			}
			else
			{
				state = 9;
			}
			break;
		}
	}while(f_continue);

	return 0;
}
