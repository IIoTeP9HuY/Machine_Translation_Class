#ifndef IBM_MODEL2_HPP
#define IBM_MODEL2_HPP

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>

#include "translation_model.hpp"
#include "alignment_model.hpp" 

class IBM_Model_2 {
public:
	std::pair<TranslationModel, AlignmentModel> train(const IntSentencePairs& sentencePairs, 
							size_t iterationsNumber,
							double defaultValue = 1.0 / 100) {
		TranslationModel translationModel(defaultValue);
		AlignmentModel alignmentModel(defaultValue);

		for (size_t iteration = 0; iteration < iterationsNumber; ++iteration) {
			std::cerr << "Iteration: " << iteration << std::endl;

			std::unordered_map<int, double> total;
			std::unordered_map<std::pair<int, int>, double> count;

			AlignmentModel countA;
			AlignmentModel totalA;

			for (size_t i = 0; i < sentencePairs.size(); ++i) {
				const auto &sentencePair = sentencePairs[i];
				size_t l_e = sentencePair.first.size(); 
				size_t l_f = sentencePair.second.size();

				std::unordered_map<int, double> sTotal;
				for (size_t i = 0; i < l_e; ++i) {
					const auto &e = sentencePair.first[i];
					sTotal[e] = 0;
					for (size_t j = 0; j < l_f; ++j) {
						const auto &f = sentencePair.second[j];
						sTotal[e] += translationModel.getTranslationProbability(e, f)
										* alignmentModel.getAlignmentProbability(i, j, l_e, l_f);
					}
				}

				for (size_t i = 0; i < l_e; ++i) {
					const auto &e = sentencePair.first[i];
					for (size_t j = 0; j < l_f; ++j) {
						const auto &f = sentencePair.second[j];

						double delta = translationModel.getTranslationProbability(e, f)
										* alignmentModel.getAlignmentProbability(i, j, l_e, l_f) / sTotal[e];

						count[std::make_pair(e, f)] += delta;
						total[f] += delta;
						countA.incAlignmentProbability(i, j, l_e, l_f, delta);
						totalA.incAlignmentProbability(0, j, l_e, l_f, delta);
					}
				}
			}	

			for (const auto &it : count) {
				translationModel.setTranslationProbability(it.first, it.second / total[it.first.second]);
			}

			for (const auto &it : countA.getAlignmentProbabilities()) {
				double probability = totalA.getAlignmentProbability(0, it.first.j, it.first.l_e, it.first.l_f);
				alignmentModel.setAlignmentProbability(it.first, it.second / probability);
			}
		}

		return std::make_pair(translationModel, alignmentModel);
	}

private:

};

std::vector< std::pair<int, int> > viterbiAlignment(const IntSentencePair &sentencePair,
									const TranslationModel &translationModel,
									const AlignmentModel &alignmentModel,
									double threshold) {
	std::vector< std::pair<int, int> > alignment;	

	size_t l_e = sentencePair.first.size();
	size_t l_f = sentencePair.second.size();

	for (size_t i = 0; i < l_e; ++i) {
		const auto &e = sentencePair.first[i];

		double maxProbability = 0.0;
		int bestAlignment = -1;

		for (size_t j = 0; j < l_f; ++j) {
			const auto &f = sentencePair.second[j];

			double probability = translationModel.getTranslationProbability(e, f)
								* alignmentModel.getAlignmentProbability(i, j, l_e, l_f);

			if (probability > maxProbability) {
				maxProbability = probability;
				bestAlignment = j;
			}
		}
		if (bestAlignment != -1 && maxProbability > threshold) {
			alignment.push_back(std::make_pair(i, bestAlignment));
		}
	}

	return alignment;
}

#endif // IBM_MODEL2_HPP