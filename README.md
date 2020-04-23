# driveSpinner
# A small program to prevent HDD from sleeping.
I am sorry to say so, but Nowadays greedy HDD producers tried to reduce durability of their products. 
That will make us to buy a new HDDs because our current HDD quickly became unusable.
For exampe one famous HDD producer add short sleep timer to all modern HDDs.
I think the intention was something about "try to save our planet through reduced power consumption".
Unfortunatelly the count of spin restore is strictly limited and HDD break down in a short period of time.
The configuration with system on SSD is the most productive to receive a broken HDD in 3 years instead of 5.
My small program will try to prevent HDD from sleeping by reading 8192 byte each 5 seconds in NOCACHE mode.
If you've got sufficient rights and trust my code then you can allow my program to read HDD directly.
Otherwise it will try to read all your disc file by file.
The code is small and the choice is yours.
