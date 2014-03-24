#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <queue>
#include <unordered_set>

#include "ibm_model1.hpp"
#include "preprocess.hpp"

using namespace std;

int sentencesNumber = 10000;

const string dataPath = "/home/acid/documents/shad/year_2/MachineTranslation/hw1/dreamt/aligner/data/";

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), 
        		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

class Aligner {
public:
	void readSentences(const string &path, int sentencesNumber) {
		ifstream eInput(path + "hansards.e");
		ifstream fInput(path + "hansards.f");

		eDict.clear();
		fDict.clear();
		sentencePairs.clear();

		size_t eTotalWords = 0;
		size_t fTotalWords = 0;
		size_t efTotalPairs = 0;

		for (size_t i = 0; i < sentencesNumber; ++i) {
			string eSentence, fSentence;
			getline(eInput, eSentence, '\n');
			getline(fInput, fSentence, '\n');

			vector<string> eTokens = tokenize(rtrim(eSentence));
			vector<string> fTokens = tokenize(rtrim(fSentence));

			eDict.addWords(eTokens);
			fDict.addWords(fTokens);

			eTotalWords += eTokens.size();
			fTotalWords += fTokens.size();
			efTotalPairs += eTokens.size() * fTokens.size();

			IntSentencePair sentencePair(std::make_pair(eDict.wordsToInts(eTokens),
														fDict.wordsToInts(fTokens)));

			sentencePairs.push_back(sentencePair);
		}

		cerr << "eDict size: " << eDict.size() << ", eWords: " << eTotalWords << endl;
		cerr << "fDict size: " << fDict.size() << ", fWords: " << fTotalWords << endl;
		cerr << "efPairs: " << efTotalPairs << endl;
	}

	void trainModel() {
		model = IBM_Model_1().train(sentencePairs, 10, 1.0 / fDict.size());
	}

	void setModel(TranslationModel &&translationModel) {
		model = translationModel;
	}

	void setModel(const TranslationModel &translationModel) {
		model = translationModel;
	}

	void saveModel(std::ostream &os) {
		for (const auto &it : model.getTranslationProbabilities()) {
			os << it.first.first << " " << it.first.second << " " << it.second << '\n';
		}
	}

	void buildAlignment(std::ostream &os, double threshold = 0.5, int neighborsNumber = 3) {
		// for (const auto &it : t.getTranslationProbabilities()) {
		// 	if (it.second < 0.5) {
		// 		continue;
		// 	}
		// 	int e = it.first.first;
		// 	int f = it.first.second;
		// 	std::cerr << eDict.getWordByIndex(e) << " " << fDict.getWordByIndex(f) << " " << it.second << std::endl;
		// }

		vector< unordered_set<int> > neighbors(eDict.size());
		vector< priority_queue< pair<double, int> > > neighborsDistances(eDict.size());

		for (const auto &sentencePair : sentencePairs) {
			for (int i = 0; i < sentencePair.first.size(); ++i) {
				for (int j = 0; j < sentencePair.second.size(); ++j) {
					int e = sentencePair.first[i];
					int f = sentencePair.second[j];
					double probability = model.getTranslationProbability(e, f);
					if (probability > threshold) {
						// cerr << i << '-' << j << ' ';
						// cerr << eDict.getWordByIndex(e) << " " << fDict.getWordByIndex(f) << " " << probability << endl;
						os << j << '-' << i << ' ';
					}
				}
			}
			os << '\n';
		}
		return;

		for (const auto &sentencePair : sentencePairs) {
			for (int i = 0; i < sentencePair.first.size(); ++i) {
				for (int j = 0; j < sentencePair.second.size(); ++j) {
					int e = sentencePair.first[i];
					int f = sentencePair.second[j];
					if (neighbors[e].find(f) == neighbors[e].end()) {
						double probability = model.getTranslationProbability(e, f);
						if (probability > threshold) {
							neighbors[e].insert(f);
							neighborsDistances[e].push(std::make_pair(probability, f));
							while (neighborsDistances[e].size() > neighborsNumber) {
								neighbors[e].erase(neighborsDistances[e].top().second);
								neighborsDistances[e].pop();
							}
						}
					}
				}
			}
		}


		for (const auto &sentencePair : sentencePairs) {
			for (int i = 0; i < sentencePair.first.size(); ++i) {
				for (int j = 0; j < sentencePair.second.size(); ++j) {
					int e = sentencePair.first[i];
					int f = sentencePair.second[j];
					if (neighbors[e].find(f) != neighbors[e].end()) {
						double probability = model.getTranslationProbability(e, f);
						// cerr << i << '-' << j << ' ';
						// cerr << eDict.getWordByIndex(e) << " " << fDict.getWordByIndex(f) << " " << probability << endl;
						os << j << '-' << i << ' ';
					}
				}
			}
			os << '\n';
		}
	}

	size_t sentencesNumber() const {
		return sentencePairs.size();
	}

private:
	IntSentencePairs sentencePairs;

	WordToIntDict eDict;
	WordToIntDict fDict;

	TranslationModel model;
};

int main(int argc, char **argv) {
	if (argc == 2) {
		sentencesNumber = stol(argv[1]);
	}

	Aligner aligner;
	aligner.readSentences(dataPath, sentencesNumber);
	// aligner.setModel(readTranslationModel("model_100000"));
	aligner.trainModel();

	ofstream modelOfs("model");
	aligner.saveModel(modelOfs);

	for (double threshold = 0.1; threshold <= 1.0; threshold += 0.1) {
		ofstream ofs("alignment.a." + to_string(int(round(threshold * 10))));
		aligner.buildAlignment(ofs, threshold, 100);
	}
	return 0;	
}