
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

#define DO_PRINT  // print the results or not to save time
//#define PRINT_ALL         // printing the full 831 vs 538
#define PRINT_DEBUG       //
#define letter_buckets 6  // a cache size 0-26, but somewhere around 4-7 is best

#define bucket_shift (26 - letter_buckets)
#define total_buckets (1 << letter_buckets)
map<char, int> letter2mask;
map<int, char> mask2letter;
map<int, vector<vector<int>>> lowhighs;
#ifdef PRINT_ALL
unordered_map<int, vector<string>> mask2words;
#else
unordered_map<int, string> mask2word;
#endif
vector<vector<int>> bucket_matcher(total_buckets);

typedef std::chrono::high_resolution_clock::time_point TimeVar;
#define duration(a) \
  std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()
void printTime(string label, TimeVar t1) {
#ifdef PRINT_DEBUG
  cout << label << " " << duration(timeNow() - t1) / 1e6 << " msec\n";
#endif
}
ofstream out("solutions.txt");

using WordArray = array<int, 5>;
struct State {
  int totalbits;
  int numwords;
  WordArray words;
  bool skipped;
  bool stop;
};
mutex queueMutex;
condition_variable queueCVar;
deque<State> queue;

void findwords(std::vector<WordArray>& solutions,
               int totalbits,
               int numwords,
               WordArray words,
               bool skipped,
               bool force = false) {
  if (numwords == 5) {
    solutions.push_back(words);
    return;
  }

  if (!force && numwords == 1) {
    {
      std::unique_lock lock{queueMutex};
      queue.push_back({.totalbits = totalbits,
                       .numwords = numwords,
                       .words = words,
                       .skipped = skipped,
                       .stop = false});
    }
    queueCVar.notify_one();
    return;
  }

  while (true) {
    int first = (totalbits + 1) & (~totalbits);
    int highs = totalbits >> bucket_shift;
    for (auto h : bucket_matcher[highs]) {
      for (auto m : lowhighs[first][h]) {
        if (totalbits & m) {
          continue;
        }
        // used[numwords] = m;
        //  srch(mask | m, single, depth + 1);
        words[numwords] = m;
        findwords(solutions, totalbits | m, numwords + 1, words, skipped);
        // findwords(solutions, totalbits | w, numwords + 1, words, i + 1,
        // skipped);
      }
    }

    if (skipped)
      break;
    skipped = true;
    totalbits |= first;
  }

  /*
    // walk over all letters in a certain order until we find an unused one
    for (int i = maxLetter; i < 26; i++) {
      int letter = letterorder[i];
      int m = 1 << letter;
      if (totalbits & m)
        continue;

      // take all words from the index of this letter and add each word to the
      // solution if all letters of the word aren't used before.
      for (int w : letterindex[i]) {
        if (totalbits & w)
          continue;

        words[numwords] = w;
        findwords(solutions, totalbits | w, numwords + 1, words, i + 1,
    skipped);

        OUTPUT(if (numwords == 0) std::cout
               << "\33[2K\rFound " << numsolutions
               << " solutions. Running time: " << (timeUS() - start) << "us");
      }

      if (skipped)
        break;
      skipped = true;
    }*/
}

void findthread(std::vector<WordArray>& solutions) {
  std::vector<WordArray> mysolutions;

  std::unique_lock lock{queueMutex};
  for (;;) {
    if (queue.empty())
      queueCVar.wait(lock, [] { return !queue.empty(); });
    State state = queue.front();
    queue.pop_front();
    if (state.stop)
      break;
    lock.unlock();
    findwords(mysolutions, state.totalbits, state.numwords, state.words,
              state.skipped, true);
    lock.lock();
  }

  solutions.insert(solutions.end(), mysolutions.begin(), mysolutions.end());
}

int findwords(std::vector<WordArray>& solutions) {
  std::vector<std::jthread> threads;
  auto numThreads = std::thread::hardware_concurrency() - 1;
  printf("threads: %d\n", numThreads);
  threads.reserve(numThreads);

  for (int i = 0; i < numThreads; i++) {
    threads.emplace_back([&]() { findthread(solutions); });
  }

  WordArray words = {};
  findwords(solutions, 0, 0, words, 0, false);

  {
    std::unique_lock lock{queueMutex};
    for (int i = 0; i < numThreads; i++)
      queue.push_back({.stop = true});
    queueCVar.notify_all();
  }
  threads.clear();

  return int(solutions.size());
}

int main() {
  TimeVar t1 = timeNow();

  // bucket_matcher cache?
  for (int high = 0; high < total_buckets; ++high) {
    for (int h = 0; h < total_buckets; ++h) {
      if ((h & high) == 0) {
        bucket_matcher[high].push_back(h);
      }
    }
  }

  int letter_rare[26];
  for (int i = 0; i < 26; ++i) {
    letter_rare[i] = 'a' + i;
  }
  vector<string> allwords;
  string filename = "words_alpha.txt";
  {
    ifstream file(filename, ios::binary | ios::ate);
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
      int linestart = 0;
      while (linestart < size) {
        int dupMask = 0;
        bool finishLine = false;
        for (int i = 0; i < 6; ++i) {
          char c = buffer[linestart + i];

          if (c == '\n') {
            if (i == 5) {
              // we have 5 letter word
              string word(buffer.begin() + linestart,
                          buffer.begin() + linestart + 5);
              // cout << linestart << word << endl;
              allwords.push_back(word);

              for (char& c : word) {
                letter_rare[c - 'a'] += 1 << 8;
              }
              linestart += 6;
              break;
            } else {
              // less than 5 letter word
              linestart += i + 1;
              break;
            }
          } else {
            if (i == 5) {
              // more than 5 letter word
              finishLine = true;
              linestart += 6;
              break;
            } else {
              // idk, still reading
              int ii = 1 << (c - 'a');
              if (dupMask & ii) {
                linestart += i + 1;
                finishLine = true;
                break;
              }
              dupMask |= ii;
            }
          }
        }
        if (finishLine) {
          while (buffer[linestart++] != '\n') {
          }
        }
      }
    }
  }

  sort(begin(letter_rare), end(letter_rare));
  for (int i = 0; i < 26; ++i) {
    int lr = letter_rare[i];
    char letter = lr & 0xff;
    int ii = 1 << i;

    letter2mask[letter] = ii;
    mask2letter[ii] = letter;
    lowhighs[ii] = vector<vector<int>>(total_buckets);
  }
  printTime("read words", t1);

  for (auto word : allwords) {
    int mask = 0;
    for (auto letter : word) {
      mask |= letter2mask[letter];
    }

#ifdef PRINT_ALL
    if (mask2words.count(mask)) {
      mask2words[mask].push_back(word);
    } else {
      mask2words[mask] = {word};
#else
    if (!mask2word.count(mask)) {
      mask2word[mask] = word;
#endif

      int low = (~(mask - 1)) & mask;    // first set bit
      int highs = mask >> bucket_shift;  // top n bits
      lowhighs[low][highs].push_back(mask);
    }
  }

#ifdef PRINT_DEBUG
  printf("words: %d   anagrams: %d\n", (int)allwords.size(),
#ifdef PRINT_ALL
         (int)mask2words.size()
#else
         (int)mask2word.size()
#endif
  );
#endif
  printTime("preprocess", t1);

  // I'm not great at multithreading yet so stealing tech from oisyn
  std::vector<WordArray> solutions;
  solutions.reserve(10000);
  int num = findwords(solutions);
  printTime("found words", t1);

#ifdef DO_PRINT
  std::ofstream out("solutions.txt");
  for (auto& words : solutions) {
#ifdef PRINT_ALL
    for (auto word0 : mask2words[words[0]]) {
      for (auto word1 : mask2words[words[1]]) {
        for (auto word2 : mask2words[words[2]]) {
          for (auto word3 : mask2words[words[3]]) {
            for (auto word4 : mask2words[words[4]]) {
              // printf("%s %s %s %s %s %c\n", word0.c_str(), word1.c_str(),
              //      word2.c_str(), word3.c_str(), word4.c_str(), single);
              out << word0 << " " << word1 << " " << word2 << " " << word3
                  << " " << word4 << endl;
            }
          }
        }
      }
    }
#else
    for (auto w : words) {
      out << mask2word[w] << " ";
    }
    out << "\n";
#endif
  };
#endif

#ifdef PRINT_DEBUG
  printf("solutions: %d \n", num);
#endif
  printTime("total", t1);

  return 0;
}