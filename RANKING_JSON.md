# Editing the Ranking JSON

The completed runs are stored in `assets/data/rankings.json`. Each entry is a JSON object inside the outer array and looks like this:

```json
{
  "id": 1700000000000,
  "playerName": "Puzzle",
  "totalSeconds": 201.45,
  "faults": 2,
  "started": "2024-08-19 20:32:45"
}
```

To remove a test run, open the file in any text editor, delete the corresponding object (including the trailing comma if it is not the last entry), and save the file.  
To add a manual record, copy an existing object, adjust the fields (`id` must be a unique integer, `playerName` the display name, etc.), and paste it inside the outer array (`[]`). Keep the JSON syntax valid (quotes around keys/strings, commas between fields/entries). After saving, the ranking list will reflect the changes the next time the game starts.

If the file contains invalid JSON, the game will ignore it and start with an empty leaderboard; simply fix the syntax to restore entries.
