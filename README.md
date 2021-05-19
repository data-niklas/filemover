# Really simple file mover
## What does it do?!
It matches files based on specified regexes and moves those matched files into specified dirs. It's a file sorter. It can also watch directories for changes and then automatically check the specified rules.

## How
- Just execute `filemover` once
- Start `filemover` with `--watch` to watch the specified dirs

## Configuration
- create a config file in `$HOME/.config/filemover/filemoverrc` in the specified format
```
[directory]
regex=directory to move to
```

Example:
Moves mp3 files into the music folder

```
[/home/user/Downloads]
.*\.mp3=/home/user/Music
```

Of course specifying several folders works too!

```
[/home/user/Downloads]
.*\.mp3=/home/user/Music

[/home/user/Music]
.*\.mp3=/home/user/Downloads
```
(But please **don't** use this sample)

## Found an error?
This programm has almost no error handling and will not improve. It will probably work if you use the specified formats above and if the specified dirs exist.<br>
