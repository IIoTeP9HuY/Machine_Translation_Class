#ifndef ALIGNMENT_MODEL_HPP
#define ALIGNMENT_MODEL_HPP

struct AlignmentKey {
	AlignmentKey(int i, int j, int l_e, int l_f): i(i), j(j), l_e(l_e), l_f(l_f) {}
	int i, j, l_e, l_f;
};

bool operator == (const AlignmentKey &lhs, const AlignmentKey &rhs) {
	return (lhs.i == rhs.i) && (lhs.j == rhs.j) 
			&& (lhs.l_e == rhs.l_e) && (lhs.l_f == rhs.l_f);
}

namespace std {
template <> struct hash<AlignmentKey> {
    inline size_t operator()(const AlignmentKey &v) const {
        std::hash<int> int_hasher;
        return int_hasher(v.i) ^ int_hasher(v.j) ^ int_hasher(v.l_e) ^ int_hasher(v.l_f);
    }
};
} // namespace std

class AlignmentModel {
public:
	explicit AlignmentModel(double defaultValue = 0.0): defaultValue(defaultValue) {
	}

	double getAlignmentProbability(const AlignmentKey &alignmentKey) const {
		auto it = alignmentProbabilities.find(alignmentKey);
		if (it == alignmentProbabilities.end()) {
			return defaultValue;
		}
		return it->second;
	}

	void setAlignmentProbability(const AlignmentKey &alignmentKey, double value) {
		alignmentProbabilities[alignmentKey] = value;
	}

	double getAlignmentProbability(int i, int j, int l_e, int l_f) const {
		return getAlignmentProbability(AlignmentKey(i, j, l_e, l_f));
	}

	void setAlignmentProbability(int i, int j, int l_e, int l_f, double value) {
		setAlignmentProbability(AlignmentKey(i, j, l_e, l_f), value);
	}

	const std::unordered_map< AlignmentKey, double> &getAlignmentProbabilities() const {
		return alignmentProbabilities;
	}

	void incAlignmentProbability(const AlignmentKey &alignmentKey, double value) {
		alignmentProbabilities[alignmentKey] += value;
	}

	void incAlignmentProbability(int i, int j, int l_e, int l_f, double value) {
		incAlignmentProbability(AlignmentKey(i, j, l_e, l_f), value);
	}

private:
	std::unordered_map< AlignmentKey, double > alignmentProbabilities;
	double defaultValue;
};

// AlignmentModel readAlignmentModel(const std::string &filename) {
// 	AlignmentModel model;
// 	std::ifstream ifs(filename);
// 	while (!ifs.eof()) {
// 		int e, f;
// 		double probability;
// 		ifs >> e >> f >> probability;
// 		model.setAlignmentProbability(e, f, probability);
// 	}
// 	return model;
// }

#endif // ALIGNMENT_MODEL_HPP