
### A solution to the problem of finding five English words with 25 distinct characters, as posed in this video by Matt Parker: https://www.youtube.com/watch?v=_-AfhLQfb6w

---
### Introduction
I came to the same algorithm everyone else did independently, but I've added a few things that help (mainly #6).  My python code uses the same algorithm as the c++, but has better comments and is easier to read.

 1. Depth first search. Pick one word at a time, check if it collides with previous word.
 2. Use bitmasks to represent the words for faster detection if a word reuses a letter. 
 3.  When picking the next word, only check words that are using the lowest bit (i.e. letter) missing.  A tiny hitch is that you have to keep track of which letter you get to skip.
 4. Store each word in separate lists based the lowest bit.  By separating them, you only have to check one list of words to use, dramatically reducing how many words to check at each layer of the depth search.
 5. Instead of putting the bits in alphabetic order, sort letters/bits based on their frequency in all words.  When combined with the above steps, this makes the search start with the hard letters to use.  Otherwise, you'll start with a word like `learn` which eats up tons of easy letters making it hard to find words that need them.
 6. As in step 4, also separate lists based on if they've used the most common letters.  This is equivalent to essentially precaching bitmask collision detections.  Out of 5977 unique bitmasks, with steps 1-5, you completely skip and never check 409 words.  With this step, 2230 are never checked directly. How many of the top bits to use affects performance.  Too many and you waste time fumbling through lists. Too few and you miss out on the sweet caching opportunities.  In my code, this is controlled by the`letter_buckets` variable/constant. Experimentally, 4-7 is pretty good.  Actual best is language and hardware dependent. 
 7. Multithreading!  I'm proud to say that my single threaded algorithm can hold it's own against [oisyn/parkerwords](https://github.com/oisyn/parkerwords) which uses multithreading.  But there's no reason this algorithm can't be multithreaded as well.  I completely stole oisyn's implementation here, so I can't claim that as my own.  It was eerie reading their code and seeing how similar it was to mine. 


Example sorted word table into separate buckets. Rows are the least common letter used. Columns are which most common letters they don't use.  This table only uses the top 3 bits.  My c++ seems to prefer 6, and python likes 5.  Other peoples' implementations effectivly use 0 and collapse each row into 1 bucket.

| | sea | ea | sa | a | se | e | s | {} | 
|--|--|--|--|--|--|--|--|--|
| q | moqui<br />quick<br />10 more | hdqrs<br />qophs<br />9 more | cequi<br />coque<br />16 more | queys<br />quest<br />2 more | antiq<br />faqir<br />20 more | qaids<br />qiyas<br />10 more | aequi<br />caque<br />9 more | sequa | 
| j | bijou<br />brujo<br />30 more | djins<br />fujis<br />20 more | bejig<br />benjy<br />43 more | ejusd<br />jebus<br />15 more | anjou<br />arjun<br />62 more | hajis<br />jacks<br />14 more | bejan<br />djave<br />19 more | hajes<br />jades<br />7 more | 
| x | boxty<br />bronx<br />25 more | nyxis<br />oxids<br />8 more | bemix<br />beryx<br />67 more | boxes<br />coxes<br />24 more | acrux<br />adfix<br />47 more | axils<br />axons<br />5 more | adnex<br />axile<br />23 more | axels<br />axers<br />6 more | 
| z | blitz<br />bortz<br />29 more | grosz<br />liszt<br />12 more | benzo<br />bezil<br />41 more | bizes<br />cozes<br />14 more | arzun<br />azido<br />58 more | azons<br />czars<br />12 more | adoze<br />adzer<br />39 more | adzes<br />fazes<br />8 more | 
| v | bovid<br />bovld<br />48 more | divus<br />intsv<br />23 more | bevil<br />bevor<br />92 more | coves<br />dives<br />32 more | admov<br />alvin<br />69 more | alvus<br />amvis<br />27 more | above<br />aevum<br />65 more | avens<br />avers<br />12 more | 
| f | clift<br />comfy<br />101 more | bumfs<br />coifs<br />77 more | befit<br />befog<br />93 more | chefs<br />clefs<br />36 more | adolf<br />afgod<br />94 more | afros<br />alifs<br />50 more | afire<br />afley<br />43 more | alefs<br />cafes<br />9 more | 
| w | blowy<br />blown<br />83 more | blows<br />brows<br />63 more | below<br />bewig<br />87 more | bowse<br />brews<br />43 more | ablow<br />adown<br />111 more | aswim<br />awols<br />57 more | alowe<br />awber<br />41 more | askew<br />awest<br />9 more | 
| k | bikol<br />birky<br />120 more | bilks<br />birks<br />105 more | becky<br />biked<br />98 more | becks<br />bikes<br />49 more | aking<br />akron<br />130 more | adusk<br />amoks<br />61 more | ackey<br />acker<br />45 more | alkes<br />asked<br />9 more | 
| b | bichy<br />bidry<br />117 more | bhuts<br />bilos<br />62 more | becry<br />becut<br />110 more | belis<br />belts<br />40 more | abdom<br />abhor<br />123 more | abysm<br />abris<br />37 more | abend<br />abide<br />55 more | abets<br />abies<br />7 more | 
| g | ching<br />chung<br />121 more | chugs<br />clogs<br />67 more | chego<br />cheng<br />94 more | doges<br />dregs<br />30 more | acing<br />agony<br />110 more | agios<br />agism<br />45 more | aegir<br />agend<br />44 more | aegis<br />agers<br />9 more | 
| p | chimp<br />chirp<br />104 more | chips<br />chops<br />80 more | chelp<br />cypre<br />92 more | copes<br />dopes<br />40 more | acoup<br />adopt<br />99 more | alisp<br />aphis<br />52 more | adept<br />ayelp<br />38 more | adeps<br />aesop<br />9 more | 
| m | chimu<br />chirm<br />75 more | chums<br />comus<br />49 more | celom<br />chime<br />74 more | cymes<br />comes<br />35 more | admin<br />admit<br />85 more | adsum<br />alums<br />37 more | admen<br />ahmed<br />37 more | acmes<br />ahems<br />8 more | 
| h | child<br />chino<br />66 more | chins<br />chits<br />47 more | cheir<br />chert<br />51 more | chest<br />choes<br />27 more | achor<br />ahind<br />65 more | alish<br />arish<br />35 more | ached<br />achen<br />23 more | aches<br />ashed<br />7 more | 
| y | cydon<br />cindy<br />40 more | cyrus<br />cloys<br />34 more | cedry<br />ceryl<br />37 more | cosey<br />desyl<br />16 more | acidy<br />acryl<br />42 more | acyls<br />ayins<br />23 more | acedy<br />aiery<br />12 more | ayens<br />asyle<br />3 more | 
| d | cloud<br />contd<br />23 more | clods<br />cords<br />29 more | cetid<br />cider<br />39 more | cedis<br />codes<br />21 more | acold<br />acrid<br />38 more | acids<br />adios<br />22 more | acned<br />acred<br />21 more | aides<br />andes<br />5 more | 
| c | clint<br />cloit<br />18 more | cions<br />clons<br />21 more | ceint<br />cento<br />20 more | ceils<br />celts<br />16 more | acoin<br />acorn<br />23 more | acost<br />actus<br />15 more | acier<br />acone<br />13 more | acies<br />acnes<br />4 more | 
| u | rutin<br />tourn<br />3 more | inust<br />irous<br />13 more | enrut<br />intue<br />13 more | etuis<br />euros<br />11 more | alout<br />altun<br />13 more | ainus<br />alnus<br />10 more | alenu<br />aleut<br />6 more | aures<br />salue<br />2 more | 
| n | inrol<br />intro<br />nilot | insol<br />instr<br />5 more | eloin<br />enrol<br />7 more | elsin<br />enols<br />7 more | aloin<br />altin<br />7 more | airns<br />anils<br />7 more | aeron<br />alien<br />7 more | aeons<br />anise<br />3 more | 
| t | lirot | islot<br />isort<br />2 more | eliot<br />lerot<br />liter | islet<br />iters<br />3 more | ariot<br />latro<br />2 more | airts<br />alist<br />4 more | alert<br />aliet<br />3 more | aotes<br />arest<br />2 more | 
| l |  | loris | oiler | liers<br />lores |  | aliso<br />arils<br />orals | ariel<br />leora | aisle<br />aloes<br />arles | 
| o |  |  |  | ireos |  | orias |  | arose | 
| i |  |  |  |  |  |  |  | aesir | 
| r |  |  |  |  |  |  |  |  | 
| s |  |  |  |  |  |  |  |  | 
| e |  |  |  |  |  |  |  |  | 
| a |  |  |  |  |  |  |  |  | 

---

### Future optimizations
- Use mmap to load the file.  I'm on Windows so I can't, but I hear it can be an additional 10% speedup.
- Multithread the file parsing too.  There's 370k words in the words list. If thread overhead isn't too much, this is easily parallelable.
- Get someone like [stew675](https://github.com/stew675/standup5x5/) or [miniBill](https://github.com/miniBill/parkerrust) to implemented it along side all of their other hyper micro optimizations. <sup><sub>hint hint, nudge nudge, wink wink</sub></sup>. 
