#/bin/bash
if ! command -v g++-13 &> /dev/null
then
    if ! command -v brew &> /dev/null
    then
        echo "Homebrew is not installed. Installing Homebrew..."
        mkdir -p $HOME/.local/share/homebrew &&
        cd $HOME/.local/share &&
        curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew &&
        eval "$(homebrew/bin/brew shellenv)" &&
        echo 'export PATH=$HOME/.local/share/homebrew/bin:$PATH' >> $HOME/.bashrc &&
        source $HOME/.bashrc
    fi
    echo "Installing dependencies..."
    brew install gcc
else
    echo "Dependencies are already installed."
fi