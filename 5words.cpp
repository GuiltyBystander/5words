
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

//#define DO_PRINT   // print the results or not to save time
//#define PRINT_ALL  // printing the full 831 vs 538
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
map<int, string> mask2word;
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

int cnt = 0, cnt2 = 0;
int used[5];
void srch(int mask, char single, int depth) {
  if (depth == 5) {
    cnt += 1;

    if (!single) {
      single = mask2letter[(mask + 1) & (~mask)];
    }

#ifdef DO_PRINT
#ifdef PRINT_ALL
    for (auto word0 : mask2words[used[0]]) {
      for (auto word1 : mask2words[used[1]]) {
        for (auto word2 : mask2words[used[2]]) {
          for (auto word3 : mask2words[used[3]]) {
            for (auto word4 : mask2words[used[4]]) {
              cnt2 += 1;
              // printf("%s %s %s %s %s %c\n", word0.c_str(), word1.c_str(),
              //      word2.c_str(), word3.c_str(), word4.c_str(), single);
              cout << word0 << " " << word1 << " " << word2 << " " << word3
                   << " " << word4 << " " << single << endl;
            }
          }
        }
      }
    }
#else
    cout << mask2word[used[0]] << " " << mask2word[used[1]] << " "
         << mask2word[used[2]] << " " << mask2word[used[3]] << " "
         << mask2word[used[4]] << " " << single << endl;
#endif
#endif

    return;
  }

  while (true) {
    int first = (mask + 1) & (~mask);
    int highs = mask >> bucket_shift;
    // for (int h = 0; h < total_buckets; ++h) {
    // if (highs & h) {
    // continue;
    //}
    for (auto h : bucket_matcher[highs]) {
      for (auto m : lowhighs[first][h]) {
        if (mask & m) {
          continue;
        }
        used[depth] = m;
        srch(mask | m, single, depth + 1);
      }
    }

    if (single) {
      return;
    }
    single = mask2letter[first];
    mask |= first;
  }
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
  printf("words: %d   anagrams: %d\n", allwords.size(),
#ifdef PRINT_ALL
         mask2words.size()
#else
         mask2word.size()
#endif
  );
#endif
  printTime("preprocess", t1);

  srch(0, 0, 0);

#ifdef PRINT_DEBUG
  printf("%d %d %d \n", cnt, cnt2);
#endif
  printTime("total", t1);

  return 0;
}