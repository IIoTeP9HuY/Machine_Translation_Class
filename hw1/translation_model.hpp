#ifndef TRANSLATION_MODEL_HPP
#define TRANSLATION_MODEL_HPP

namespace std {
template <> struct hash<std::pair<int, int>> {
    inline size_t operator()(const std::pair<int, int> &v) const {
        std::hash<int> int_hasher;
        return int_hasher(v.first) ^ int_hasher(v.second);
    }
};
} // namespace std

typedef std::pair< std::vector<int>, std::vector<int> > IntSentencePair;
typedef std::vector<IntSentencePair> IntSentencePairs;

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

#endif // TRANSLATION_MODEL_HPP