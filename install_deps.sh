#/bin/bash
mkdir -p $HOME/.local/share/homebrew &&
cd $HOME/.local/share &&
curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew &&
eval "$(homebrew/bin/brew shellenv)" &&
echo 'export PATH=$HOME/.local/share/homebrew/bin:$PATH' >> $HOME/.bashrc &&
source $HOME/.bashrc &&
brew install gcc
