#ifndef PREPROCESS_HPP
#define PREPROCESS_HPP

#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>

class WordToIntDict {
public:
	WordToIntDict() {
	}

	WordToIntDict(const std::vector<std::string> &words) {
		addWords(words);
	}

	int addWord(const std::string &word) {
		if (wordToInt.find(word) != wordToInt.end()) {
			return wordToInt[word];
		}

		int index = wordToInt.size();
		wordToInt[word] = index;
		intToWord[index] = word;
		return index;
	}

	void addWords(const std::vector<std::string> &words) {
		for (const auto &word : words) {
			(void) addWord(word);
		}
	}

	int getWordIndex(const std::string &word) const {
		return wordToInt.at(word);	
	}

	std::string getWordByIndex(int index) const {
		return intToWord.at(index);
	}

	std::vector<int> wordsToInts(const std::vector<std::string> &words) {
		std::vector<int> ints;
		for (const auto &word : words) {
			ints.push_back(getWordIndex(word));
		}
		return ints;
	}

	size_t size() const {
		return wordToInt.size();
	}

	void clear() {
		wordToInt.clear();	
		intToWord.clear();
	}

private:
	std::unordered_map<std::string, int> wordToInt;
	std::unordered_map<int, std::string> intToWord;
};

std::vector<std::string> tokenize(std::string sentence) {
	std::vector<std::string> tokens;	
	std::istringstream ss(sentence);

	while (!ss.eof()) {
		std::string token;
		getline(ss, token, ' ');
		tokens.push_back(token);
	}
	return tokens;
}

#endif // PREPROCESS_HPP