# Generation IV Mystery Gift file structures

This document aims to describe the files used in Mystery Gift transmission for the Generation IV Pokemon games.

When making a Mystery GIft, IVgift generates a zip download with 3 main files.

## PGT file

- Stands for 'Pokemon Gift'.
- 260 bytes.
- Stores the in-game data to be received by the player.

### Structure (in byte ranges)

000->001 - Gift type

004->005 - Gift type specific data

008->0f3 - .ek4 Pokemon data

0f4->103 - Unknown, speculate it is the Gift distributor ID, does not affect Mystery Gift.

## PCD file

- Stands for 'Pokemon Card Data'.
- 856 bytes.
- Stores the PGT along with visual data about the associated Wonder Card.

### Structure (in byte ranges)

000->103 - PGT data

104->14b - Wonder Card title

14c->14d - Game availability flags

150->151 - Wonder Card ID

152->153 - Mystery Bytes

154->347 - Wonder Card description

348->349 - Wonder Card distribution count

34a->34b - Pokemon Icon Left

34c->34d - Pokemon Icon Middle

34e->34f - Pokemon Icon Right

355->356 - Distribution date

## MYG file

- Stands for 'Mystery Gift'.
- 936 bytes.
- A wrapper for the PCD data. This is the main file sent out during Mystery Gift transmission.

### Structure

As mentioned, a MYG file is a wrapper for the PCD data.

Bytes 104->154 of the PCD file make up the header of the MYG file,
and the rest of the body is just the PCD file.
