from time import time
import itertools

start = time()

# pick a word list
filename = 'words_alpha.txt'
#filename = 'valid-wordle-words.txt'


ascii_offset = ord('a')  # cache this value for speed
allwords = []
letter_rare = [[0, chr(ascii_offset+i)] for i in range(26)]
with open(filename) as file:
    for line in file:
        # normally I'd do line.strip(), but that's slower
        # new line is the 6th letter
        if len(line) != 6:  # 5 letter words
            continue
        if len(set(line)) != 6:  # 5 unique letters
            continue
        word = line[:5]

        # for now we just store each word and count letter frequency
        allwords.append(word)
        for letter in word:
            letter_rare[ord(letter)-ascii_offset][0] += 1

# now that we have letter frequency, we'll be sorting each word based on
#  * it's rarest letter
#  * if it has the 5 most common letters (5 was picked experimentally)

# looking at top 5 common letters.
# caching some commonly used numbers/tables for it
letter_buckets = 5
bucket_shift = 26-letter_buckets
total_buckets = 1 << letter_buckets
bucket_matcher = [[h for h in range(total_buckets) if h & high == 0]
                  for high in range(total_buckets)]

# sort letters and assign masks based on rarity
letter_rare.sort()
letter2mask = dict()
mask2letter = dict()
lowhighs = dict()
for i, v in enumerate(letter_rare):
    letter = v[1]
    ii = 1 << i

    letter2mask[letter] = ii
    mask2letter[ii] = letter
    lowhighs[ii] = [[] for i in range(total_buckets)]

print('read words', time()-start)


# build a bitmask for each word
# and store the word in a bucket based on it's first bit and top few bits
mask2word = dict()
for word in allwords:
    mask = 0
    for letter in word:
        mask |= letter2mask[letter]

    if mask in mask2word:
        mask2word[mask].append(word)
    else:
        mask2word[mask] = [word]

        low = (~(mask-1)) & mask  # first set bit
        highs = mask >> bucket_shift  # top n bits
        lowhighs[low][highs].append(mask)

print('words:', len(allwords), 'anagrams:', len(mask2word))
print('preprocess', time()-start)

# for i in range(26):
#    print(i, letter_rare[i])
# sys.exit()


cnt = 0
cnt2 = 0
cnt3 = 0
used = [0]*5


def search(mask=0, single=False, depth=0):
    global cnt, cnt2, cnt3, start
    # cnt3[depth]+=1
    cnt3 += 1
    if depth == 5:
        # yay solution
        cnt += 1

        if not single:
            # find the missing letter if we haven't skipped one already
            single = mask2letter[(mask+1) & (~mask)]

        anagrams = [mask2word[u] for u in used]
        for word5 in itertools.product(*anagrams):
            cnt2 += 1
            print(*word5, single)
            #print('{:.3f}'.format(time()-start), *word5, single)
        return

    while True:
        # find the first unset bit
        first = (mask+1) & (~mask)
        highs = mask >> bucket_shift
        # try on that letter
        for h in bucket_matcher[highs]:
            for m in lowhighs[first][h]:
                if mask & m:
                    continue
                used[depth] = m
                search(mask | m, single, depth+1)

        # if we've already used our freebie, return
        if single:
            return
        # mark this letter as our freebie
        single = mask2letter[first]
        mask |= first


search()
print('{:.3f}'.format(time()-start), cnt, cnt2, cnt3)
