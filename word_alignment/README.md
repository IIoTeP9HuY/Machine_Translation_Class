# Word alignment task

This assignment is designed to get you to understand the problem of word alignment in practice.
You are provided with some data, some scripts and some baseline word alignment (described below).

The data consists of:
* Parallel tokenized texts in English and Czech.
* Some manually word aligned data (dev, test and blind - well I've actually removed the alignments from the blind set but if you provide your own I'll evaluate them).
* Linguist analysis of the parallel text:
  (a) Penn Tree bank tags for English
  (b) Lemmas, Prague Tree bank tags and detailled tags (morphological analysis) for Czech.
* Some additional tokenized parallel texts (English and Slovak and English and Slovene).

The scripts consist of:
* corpus_reader.py: A tool for looking at word alignments on the command line.
* eval.py: A tool for evaluating a candidate word alignment against a reference.
* align.py: A very crude word alignment tool.

The baselines consist of word alignments obtained from GIZA++ (TODO).
* file:test.tar.gz (test corpora and word alignments)
* file:small.tar.gz (smallish training corpora - includes test data at the start)
* file:scripts.tar.gz
* file:all.tar.gz (same as small but not)
* file:additionallanguages.tar.gz (parallel corpora in related languages)
