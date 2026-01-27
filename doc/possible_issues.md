## List of cast server exploits
### Packet replicas

- If you send the kill packet on the person thats not in the same room as you there is 2 options for what can happen - either random person dies, or room closes
-- possible fix: TBA

- Sending end match packets multiple times => level up exploit
-- Possible fix: Check if (hasMatchStarted)

- Single wave rewards duping (resending client=>server "won" packet) => multiple rewards
-- Possible fix: TBA.

- Shield, power, and other items => one can exploit this and have them indefinitely
-- Possible fix: for each item, make sure it's saved in the server when it's retrieved
-- Then when the user tries to use the item: check whether they really have it. After use => remove from server

- Capsule (in-match) exploits => Define whether it's automatic pick-up or just respawning multiple of them
-- Possible fix: TBA

- Spamming people (which crashes them) => chat whisper spam or normal spam
-- Possible fix: chat filtering, timeouts
- Insta killing someone even if they have spawn shields => just send kill packet replica
-- Possible fix: TBA
- Having infinite HP => via taking HP item & packet replica
-- Possible fix: TBA
- Infinite ammo and shield => via taking ammo / shield & packet replica
-- Possible fix: TBA

- Killing someone while not inside the match, or killing someone from your team
  -- Possible fix: Check that both targets are inside the match
  -- Possible fix: check that both targets aren't in the same team

### Probably? possible cheats to "detect" server side
#### blatant speed hacking (example: cheat engine builtin speed hack)
Parameters needed: SessionID, player position

#### general packet flooding
Examples:
- room rape
- explosives flooding
- tba
  
#### jump hacks? (may have false positives with normal jumps from high places)
Parameters needed: SessionID, player position

#### fly hacks? (may have same false positives)
Parameters needed: SessionID, player position

#### multiple-kill hacks / "room rape" (need to define how)
Parameters needed: SessionID, player kill action => some how we need to check time passed since last kill action to check if there are multiple kill actions in a very short period

#### resending the kill packet (hack where you kill a chosen target multiple times) (need to define how)
Parameters needed: SessionID, not sure what else...

-- may be "fixed" by not allowing identical packets in a window of ~5 minutes (unless header-only)
- Maybe try to flag range hacks? (Could have false positives, TBA?)
- Maybe try to track sniper & rifle accuracy if possible?

## Possible general solutions
- Packet rate limit (issue: player position has many natural packets)
- For some packets, we can assume that they won't be identical in a given timeframe
