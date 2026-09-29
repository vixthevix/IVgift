# Generation IV Mystery Gift file structures

This document aims to describe the files used in Mystery Gift transmission for the Generation IV Pokemon games.

When making a Mystery GIft, IVgift generates a zip download with 3 main files.

## PGT file

- Stands for 'Pokemon Gift'.
- 260 bytes.
- Stores the in-game data to be received by the player.

### Structure (in byte ranges)

``` plain
000->001 - Gift Type
004->005 - Gift Type Specific Data

These values have been documented on Project Pokemon as the following:

 -------------------------------------------------------- 
| Gift Type | Gift Type       | Gift Type                |
| Value     | Meaning         | Specific Data            |
|-----------+-----------------+--------------------------|
| 0x0000    | None            | None                     |
| 0x0100    | Pokemon         | Receiver is OT flag      |
| 0x0200    | Pokemon Egg     | Receiver is OT flag      |
| 0x0300    | Item            | In-game item index       |
| 0x0400    | Rule (???)      | ???                      |
| 0x0500    | Seal            | In-game seal index (???) |
| 0x0600    | Accessory       | In-game accessory index  |
| 0x0700    | Manaphy Egg     | Unused                   |
| 0x0800    | Member Card     | Unused                   |
| 0x0900    | Oak's Letter    | Unused                   |
| 0x0a00    | Azure Flute     | Unused                   |
| 0x0b00    | Poketch App     | App index (???)          |
| 0x0c00    | Secret Key      | Unused                   |
| 0x0d00    | ???             | ???                      |
| 0x0e00    | PokeWalker Area | Area index (???)         |
 -------------------------------------------------------- 

IVgift currently only allows the creation of
    - Pokemon
    - Pokemon Eggs
    - Items

More to be added in the future as more research is conducted. 


008->0f3 - .ek4 Pokemon data

This is the encrypted format used for a Pokemon. 
It is used in the creation of a Pokemon (Egg) that is sent to the games.
236 bytes.


0f4->103 - Unknown

I speculate it is the Gift distributor ID from back when these events were being distributed. 
Does not affect Mystery Gift.
```

## PCD file

- Stands for 'Pokemon Card Data'.
- 856 bytes.
- Stores the PGT along with visual data about the associated Wonder Card.

### Structure (in byte ranges)

``` plain
000->103 - PGT data

Just a copy of the PGT data, see above.


104->14b - Wonder Card title

Uses the Generation IV Character Encoding (see 'More Information').
36 character limit.
Not all characters supported, needs further research.


14c->14d - Game availability flags

These are used to indicate which games can receive this Wonder Card.

 ---------------------------------------------------------------------------------------
| Bits  | 15 | 14 | 13 | 12 | 11 | 10 | 9  | 8  | 7  | 6  | 5  | 4  | 3  | 2  | 1  | 0  |
|-------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|
| Flags | HG | *  | *  | *  | *  | *  | *  | *  | *  | *  | *  | Pt | P  | D  | *  | SS |
 ---------------------------------------------------------------------------------------


150->151 - Wonder Card ID

A value assigned to the Wonder Card for in-game identification.
I believe the games will check for duplicate Wonder Cards by checking this value, though untested.
Range is 0 -> 2046.


152->153 - Mystery Bytes

These 2 bytes do not affect the Mystery Gift from research so far.
In most official distribution files, these bytes are put as 0x0d00.
More research required.


154->347 - Wonder Card description

Uses the Generation IV Character Encoding (see 'More Information').
250 character limit.


348->349 - Wonder Card distribution count

Indicates the number of times this Wonder Card can be received by a game.
Range is 0 -> 255, where 255 indicates that the Wonder Card has an unlimited distribution count.


34a->34b - Pokemon Icon Left
34c->34d - Pokemon Icon Middle
34e->34f - Pokemon Icon Right

Visual Pokemon icons on the top-left of the Wonder Card front. They are indexed by Pokedex number.
0 means no Pokemon is displayed, and there are no sprites for alternate forms (such as the Deoxys formes or the Unown alphabet).


355->356 - Distribution date

The number of days of a specified date since the Nintendo DS Epoch (1/1/2000).
When received in-game, the date is changed to the DS's current system date.
```

## MYG file

- Stands for 'Mystery Gift'.
- 936 bytes.
- A wrapper for the PCD data. This is the main file sent out during Mystery Gift transmission.

### Structure

As mentioned, a MYG file is a wrapper for the PCD data.

Bytes 0x104->0x154 of the PCD file make up the header of the MYG file,
and the rest of the body is just the PCD file.

## More Information

### Links

[Project Pokemon PGT documentation](https://projectpokemon.org/home/docs/gen-4/pgt-file-r35/)
<br>
[Generation IV Character Set](https://bulbapedia.bulbagarden.net/wiki/Character_encoding_(Generation_IV))
