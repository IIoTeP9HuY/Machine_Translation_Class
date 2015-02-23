#!/usr/bin/python
import sys, codecs

def read_all_tokens(path):
    return [line.strip().split() for line in codecs.open(path, 'r', 'utf8')]

def parse_alignment_line(line, src_sentence, trg_sentence):
    alignments = {}
    assert line.startswith("NULL ({") and line.strip().endswith("})")
    tokens = line.strip().split("})")
    for i, token in enumerate(tokens[:-1]):
        src_word = token.split("({")[0].strip()
        if src_word == "NULL" and i == 0:
            continue
        src_index = i - 1
        if src_index not in alignments:
            alignments[src_index] = {}
        assert src_index >= 0 and src_index < len(src_sentence)
        assert src_word == src_sentence[src_index], "%s != %s. token=%s; line=%s" % (src_word, src_sentence[src_index], token, line.strip())
        trg_index_str = token.split("({")[1].strip()
        trg_indices = [int(index) - 1 for index in trg_index_str.split()]
        for trg_index in trg_indices:
            assert trg_index >= 0
            assert trg_index < len(trg_sentence)
            alignments[src_index][trg_index] = "*"
    return alignments

def process_giza_alignment_file(src_corpus, trg_corpus, giza_alignments):
    sent_index = 0
    lines = [line.strip() for line in codecs.open(giza_alignments, 'r', 'utf8')]
    # giza occasionally doesn't output alignments for all sentences
    if len(lines) != len(src_corpus) * 3:
        return [{} for i in range(len(src_corpus))]
    alignments = []
    for line in lines:
        if line.startswith("# Sentence pair"):
            continue
        if line.startswith("NULL ({"):
            these_alignments = parse_alignment_line(
                line.strip(), src_corpus[sent_index], trg_corpus[sent_index])
            sent_index += 1
            alignments.append(these_alignments)
    return alignments


if __name__ == "__main__":
    if not len(sys.argv) == 4:
        print "Usage: ./giza_to_standard.py src_corpus trg_corpus giza_alignments"
        sys.exit(0)
    src_corpus = read_all_tokens(sys.argv[1])
    trg_corpus = read_all_tokens(sys.argv[2])
    alignments = process_giza_alignment_file(src_corpus, trg_corpus, sys.argv[3])
    for i, alignment in enumerate(alignments):
        line = ""
        for src_index in alignment:
            for trg_index in alignment[src_index]:
                if len(line):
                    line += " "
                line += "%d-%d-*" % (src_index, trg_index)
        print line
