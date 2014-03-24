#ifndef IBM_MODEL1_HPP
#define IBM_MODEL1_HPP

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>

#include "translation_model.hpp"

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
					} else {
						for (const auto &it : threadTotal) {
							total[it.first] += it.second;
						}
						for (const auto &it : threadCount) {
							count[it.first] += it.second;
						}
					}
				}
			}

			for (const auto &it : count) {
				translationModel.setTranslationProbability(it.first, it.second / total[it.first.second]);
			}
		}

		return translationModel;
	}

private:

};

#endif // IBM_MODEL1_HPP