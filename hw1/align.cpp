#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <queue>
#include <unordered_set>

#include "ibm_model1.hpp"
#include "ibm_model2.hpp"
#include "preprocess.hpp"

using namespace std;

int sentencesNumber = 10000;

const string dataPath = "../data/";

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), 
        		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

class Aligner {
public:
	Aligner(): translationModel(1.0), alignmentModel(1.0) {
	}

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

	template<typename ModelBuilder>
	void trainModel(const ModelBuilder& modelBuilder) {
		std::tie(translationModel, alignmentModel) = modelBuilder.train(sentencePairs,
																		translationModel,
																		alignmentModel);
	}

	void setTranslationModel(TranslationModel &&translationModel) {
		this->translationModel = translationModel;
	}

	void setTranslationModel(const TranslationModel &translationModel) {
		this->translationModel = translationModel;
	}

	void setAlignmentModel(AlignmentModel &&alignmentModel) {
		this->alignmentModel = alignmentModel;
	}

	void setAlignmentModel(const AlignmentModel &alignmentModel) {
		this->alignmentModel = alignmentModel;
	}

	void buildAlignmentModel2(std::ostream &os, double threshold = 0.5) {
		size_t processed = 0;
		size_t blockSize = ((sentencePairs.size() + 9) / 10);
		for (const auto &sentencePair : sentencePairs) {
			if (processed % blockSize == 0) {
				cerr << "Processed " << processed << endl;
			}
			auto alignment = viterbiAlignment(sentencePair, translationModel, alignmentModel, threshold);
			for (const auto &alignmentPair : alignment) {
				os << alignmentPair.second << "-" << alignmentPair.first << " ";
			}
			os << "\n";
			++processed;
		}
	}

	void buildAlignmentModel1(std::ostream &os, double threshold = 0.5) {
		size_t processed = 0;
		size_t blockSize = ((sentencePairs.size() + 9) / 10);

		for (const auto &sentencePair : sentencePairs) {
			if (processed % blockSize == 0) {
				cerr << "Processed " << processed << endl;
			}
			for (int i = 0; i < sentencePair.first.size(); ++i) {
				for (int j = 0; j < sentencePair.second.size(); ++j) {
					int e = sentencePair.first[i];
					int f = sentencePair.second[j];
					double probability = translationModel.getTranslationProbability(e, f);
					if (probability > threshold) {
						os << j << '-' << i << ' ';
					}
				}
			}
			os << '\n';
			++processed;
		}
	}

	size_t sentencesNumber() const {
		return sentencePairs.size();
	}

	TranslationModel &getTranslationModel() {
		return translationModel;
	}

	AlignmentModel &getAlignmentModel() {
		return alignmentModel;
	}

private:
	IntSentencePairs sentencePairs;

	WordToIntDict eDict;
	WordToIntDict fDict;

	TranslationModel translationModel;
	AlignmentModel alignmentModel;
};

int main(int argc, char **argv) {
	if (argc == 2) {
		sentencesNumber = stol(argv[1]);
	}

	Aligner aligner;
	aligner.readSentences(dataPath, sentencesNumber);
	// aligner.setModel(readTranslationModel("translationModel_100000"));

	aligner.trainModel(IBM_Model_1(10));
	// aligner.getTranslationModel().saveToFile("model_100000");

	// aligner.getTranslationModel().loadFromFile("model_100000");
	aligner.trainModel(IBM_Model_2(1));

	// aligner.getTranslationModel().saveToFile("translation_model");
	// aligner.getAlignmentModel().saveToFile("alignment_model");
	

	ofstream ofs("alignment.a");
	aligner.buildAlignmentModel2(ofs, 0.1);

	// for (double threshold = 0.1; threshold <= 1.0; threshold += 0.1) {
	// 	ofstream ofs("alignment.a." + to_string(int(round(threshold * 10))) + ".model2");
	// 	aligner.buildAlignmentModel2(ofs, threshold);
	// }
	return 0;	
}