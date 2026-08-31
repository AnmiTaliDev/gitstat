# gitstat

## About

gitstat is a command-line tool for querying and displaying profile statistics and recent activity from GitHub and GitLab.

## Dependencies

- C23 compliant C compiler (GCC 14+ or Clang 18+)
- Meson build system (1.0+)
- Ninja
- libcurl

## Build

```sh
meson setup build
ninja -C build
```

The resulting executable is located at `build/gitstat`.

## Usage

```sh
./build/gitstat [OPTIONS] [USERNAME]
```

Options:
- `-u, --user <name>`: Target user login
- `-p, --provider <name>`: Provider (`github` or `gitlab`)
- `-t, --token <token>`: API authentication token
- `--url <url>`: API endpoint URL
- `-l, --limit <n>`: Number of recent repositories to show
- `-c, --config <file>`: Path to configuration file
- `--color <mode>`: Color mode (`auto`, `always`, `never`)
- `--json`: Output raw JSON stats
- `-h, --help`: Show help information
- `-v, --version`: Show version information

## Configuration

gitstat looks for a configuration file in the following locations:
- `$XDG_CONFIG_HOME/gitstat/config.ini`
- `~/.config/gitstat/config.ini`
- `~/.gitstatrc`
- `%APPDATA%\gitstat\config.ini` (Windows)

Example `config.ini`:

```ini
[general]
provider = github
user = username
color = auto
repo_limit = 3
timeout = 10

[github]
token =
url = https://api.github.com/graphql

[gitlab]
token =
url = https://gitlab.com/api/graphql

[display]
show_contributions = true
show_commits = true
show_pull_requests = true
show_restricted = true
show_public_repos = true
show_recent_repos = true
```

## Acknowledgments

- GitHub GraphQL and REST APIs
- GitLab REST and GraphQL APIs
- cURL library project

## Links to docs and CONTRIBUTING

- [CONTRIBUTING.md](CONTRIBUTING.md)

## License

MIT License
