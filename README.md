# Vanilla+ Upgraded Init.c with Commands
## KillFeed, Stats, Global chat, Moderation and Player Database.
| Github | [johnnypush/DayZVanillaInit][PlDb] |
```diff
-  No mods required
```
I will no longer be managing DayZ servers, so I decided to gift my work for the community.
So feel free to share, credits appreciated.

## Features

- ##### KillFeed

> Whenever two Players have the KillFeed enabled and they kill each other,
the server will send a message to all players with the following message:

```diff
-  [[KillFeed]] Vladimir was killed by Swain with BK-18.
```

> By default, every player will have the KillFeed disabled.
Only if both players have the KillFeed enabled the Killfeed will be visible 

> Commands to enable or disable the KillFeed:

 ```
 /killfeed on
 /killfeed off
```

> Commands are applied few minutes after they are requested to prevent abuse.

- ##### Stats

> Whenever a Player kills or dies by other player, stats are updated (few minutes after the event).

> Players can always check their stats using the following command:

```
 /stats
```
> The server will then send a message to the Player as the following example:

```diff
-  [[Stats]] Name: Vladimir | Kills: 2 | Deaths: 1 | K/D Ratio: 2.0 | KillFeed Status: Enabled
```
- ##### Global chat

> Players will have global chat enabled by default. 
> They can always switch the status of it with the following commands.
```
 /chat global
 /chat local
```
- ##### Moderation

> Mods mute players from the global chat, permanently or for some minutes

> In order to mute Player Vladimir permanently:
```
 ?mute perm Vladimir
```
>To Enable Player Vladimir global chat:
```
 ?mute disable Vladimir
```
>To mute Player Vladimir for 10 minutes (or until server restart).
```
 ?mute 10 Vladimir
```
>To mute Player Vladimir for 2 hours (or until server restart).

```
 ?mute 120 Vladimir
```

## System Information
> Players without name (Survivors), won't have access to any command, they will have the plain vanilla experience.
> Player Names can't be duplicated, if a player logs in with a name that already exists in the database,
the systegm will remind the player to change the name and will be kicked.

## Configuration

##### Attached to this repository you will find different config files:
These files will be automatically generated by the init.c file under the following locations:
```
DayZServer\config\adminConfig.json
DayZServer\config\sysConfig.json
DayZServer\config\playersConfig.json
```
- sysConfig.json
>This configuration file will let you configure different how often events occur.
If you don't want the server to kick Players that have a name that already exists on the system,
change "kickPlayersOnDuplicateName" to "0".
```
{
    "autoSaveMinutes": "4",
    "kickPlayersOnDuplicateName": "1",
    "debug_enabled": "0",
    "serverRestartMinutes": "384",
    "killFeedNotifyMinutes": "1"
}
```

- playersConfig.json
>This is our custom player database, where every player stat will be stored,
it is highly recommended to periodically backup this file.
It will have the following structure:
```
{
    "VXnOuYpHqFaaaaaL0dpsEtJnlp3WI9nJdZoTtb8xf-E=": {
        "deaths": "2",
        "killFeedOn": "true",
        "kills": "0",
        "localChat": "false",
        "lastConnection": "2023-02-14",
        "name": "Swain"
    },
    "yUTZ5Pjjaaaaaa3KyVVZAtJ4gsftQSiDxhXeHESCEUs=": {
        "name": "Jinx",
        "killFeedOn": "false",
        "localChat": "false",
        "kills": "0",
        "lastConnection": "2023-03-03",
        "deaths": "0"
    },
    "wGx7jraaaaaaTC17nqdWKjVNT4POtOqWCZc1ktVpi84=": {
        "deaths": "1",
        "killFeedOn": "true",
        "kills": "3",
        "localChat": "false",
        "lastConnection": "2023-03-03",
        "name": "Singed"
    }
}
```
- adminConfig.json
>This configuration file will let you add mods to the system, in order to do so, you have to
manually add the bohemiaInteractiveID to the modPlayers as shown in the following example.
You can also permanently mute players by adding the bohemiaInteractiveID to the mutedPlayers.
Note: If you don't know the bohemiInteractiveID, you can extract it from the playersConfig.json
```
{
    "modPlayers": "VXnOuYpHqFaaaaaL0dpsEtJnlp3WI9nJdZoTtb8xf yUTZ5Pjjaaaaaa3KyVVZAtJ4gsftQSiDxhXeHESCEUs",
    "adminPlayers": "",
    "mutedPlayers": "wGx7jraaaaaaTC17nqdWKjVNT4POtOqWCZc1ktVpi84"
}
```
>With this example, Swain and Jinx will be mods and Singed will be muted.

[//]: # (These are reference links)

   [PlDb]: <https://github.com/johnnypush/DayZVanillaInit>
