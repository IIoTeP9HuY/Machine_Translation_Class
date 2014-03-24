#ifndef IBM_MODEL1_HPP
#define IBM_MODEL1_HPP

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>

typedef std::pair< std::vector<int>, std::vector<int> > IntSentencePair;
typedef std::vector<IntSentencePair> IntSentencePairs;

namespace std {
template <> struct hash<std::pair<int, int>> {
    inline size_t operator()(const std::pair<int, int> &v) const {
        std::hash<int> int_hasher;
        return int_hasher(v.first) ^ int_hasher(v.second);
    }
};
} // namespace std

class TranslationModel {
public:
	explicit TranslationModel(double defaultValue = 0.0): defaultValue(defaultValue) {
	}

	double getTranslationProbability(const std::pair<int, int> &wordPair) const {
		auto it = translationProbabilities.find(wordPair);
		if (it == translationProbabilities.end()) {
			return defaultValue;
		}
		return it->second;
	}

	void setTranslationProbability(const std::pair<int, int> &wordPair, double value) {
		translationProbabilities[wordPair] = value;
	}

	double getTranslationProbability(int firstWord, int secondWord) const {
		return getTranslationProbability(std::make_pair(firstWord, secondWord));
	}

	void setTranslationProbability(int firstWord, int secondWord, double value) {
		translationProbabilities[std::make_pair(firstWord, secondWord)] = value;
	}

	const std::unordered_map< std::pair<int, int>, double> &getTranslationProbabilities() const {
		return translationProbabilities;
	}

private:
	std::unordered_map< std::pair<int, int>, double > translationProbabilities;
	double defaultValue;
};

TranslationModel readTranslationModel(const std::string &filename) {
	TranslationModel model;
	std::ifstream ifs(filename);
	while (!ifs.eof()) {
		int e, f;
		double probability;
		ifs >> e >> f >> probability;
		model.setTranslationProbability(e, f, probability);
	}
	return model;
}

class IBM_Model_1 {
public:
	TranslationModel train(const IntSentencePairs& sentencePairs, 
							size_t iterationsNumber,
							double defaultValue = 1.0 / 100) {
		TranslationModel translationModel(defaultValue);

		for (size_t iteration = 0; iteration < iterationsNumber; ++iteration) {
			std::cerr << "Iteration: " << iteration << std::endl;

			std::unordered_map<int, double> total;
			std::unordered_map<std::pair<int, int>, double> count;

			#pragma omp parallel shared(total, count)
			{
				std::unordered_map<int, double> threadTotal;
				std::unordered_map<std::pair<int, int>, double> threadCount;

				#pragma omp for schedule(static)
				for (size_t i = 0; i < sentencePairs.size(); ++i) {
					const auto &sentencePair = sentencePairs[i];

					std::unordered_map<int, double> sTotal;
					for (const auto &e : sentencePair.first) {
						sTotal[e] = 0;
						for (const auto &f : sentencePair.second) {
							sTotal[e] += translationModel.getTranslationProbability(e, f);
						}
					}

					for (const auto &e : sentencePair.first) {
						for (const auto &f : sentencePair.second) {
							double delta = translationModel.getTranslationProbability(e, f) / sTotal[e];
							threadCount[std::make_pair(e, f)] += delta;
							threadTotal[f] += delta;
						}
					}
				}	

				#pragma omp critical
				{
					if (total.empty() && count.empty()) {
						std::swap(total, threadTotal);
						std::swap(count, threadCount);
					}

					for (const auto &it : threadTotal) {
						total[it.first] += it.second;
					}
					for (const auto &it : threadCount) {
						count[it.first] += it.second;
					}
				}
			}
			// for (const auto &sentencePair : sentencePairs) {
			// 	std::unordered_map<int, double> sTotal;
			// 	for (const auto &e : sentencePair.first) {
			// 		sTotal[e] = 0;
			// 		for (const auto &f : sentencePair.second) {
			// 			sTotal[e] += translationModel.getTranslationProbability(e, f);
			// 		}
			// 	}

			// 	for (const auto &e : sentencePair.first) {
			// 		for (const auto &f : sentencePair.second) {
			// 			double delta = translationModel.getTranslationProbability(e, f) / sTotal[e];
			// 			count[std::make_pair(e, f)] += delta;
			// 			total[f] += delta;
			// 		}
			// 	}
			// }

			for (const auto &it : count) {
				translationModel.setTranslationProbability(it.first, it.second / total[it.first.second]);
			}
		}

		return translationModel;
	}

private:

};

#endif // IBM_MODEL1_HPP