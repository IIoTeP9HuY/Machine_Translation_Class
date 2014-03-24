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
	IBM_Model_2(size_t iterationsNumber): iterationsNumber(iterationsNumber) {
	}

	std::pair<TranslationModel, AlignmentModel> train(const IntSentencePairs& sentencePairs, 
	                                                  TranslationModel translationModel,
	                                                  AlignmentModel alignmentModel,
	                                                  size_t sentencesNumber = std::numeric_limits<size_t>::max()) const {

		sentencesNumber = std::min(sentencesNumber, sentencePairs.size());
		for (size_t iteration = 0; iteration < iterationsNumber; ++iteration) {
			std::cerr << "Iteration: " << iteration << std::endl;

			std::unordered_map<int, double> total;
			std::unordered_map<std::pair<int, int>, double> count;

			AlignmentModel countA;
			AlignmentModel totalA;

			#pragma omp parallel shared(total, count, countA, totalA)
			{
				std::unordered_map<int, double> threadTotal;
				std::unordered_map<std::pair<int, int>, double> threadCount;
				AlignmentModel threadCountA;
				AlignmentModel threadTotalA;

				#pragma omp for schedule(static)
				for (size_t k = 0; k < sentencesNumber; ++k) {
					const auto &sentencePair = sentencePairs[k];
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

							threadCount[std::make_pair(e, f)] += delta;
							threadTotal[f] += delta;
							threadCountA.incAlignmentProbability(i, j, l_e, l_f, delta);
							threadTotalA.incAlignmentProbability(0, j, l_e, l_f, delta);
						}
					}
				}

				#pragma omp critical
				{
					if (total.empty() && count.empty()) {
						std::swap(total, threadTotal);
						std::swap(count, threadCount);
						std::swap(countA, threadCountA);
						std::swap(totalA, threadTotalA);
					} else {
						for (const auto &it : threadTotal) {
							total[it.first] += it.second;
						}
						for (const auto &it : threadCount) {
							count[it.first] += it.second;
						}
						for (const auto &it : threadTotalA.getAlignmentProbabilities()) {
							totalA.incAlignmentProbability(it.first, it.second);
						}
						for (const auto &it : threadCountA.getAlignmentProbabilities()) {
							countA.incAlignmentProbability(it.first, it.second);
						}
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
	size_t iterationsNumber;
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