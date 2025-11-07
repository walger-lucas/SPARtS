echo "====> Installing Application Frontend Dependencies <===="
echo ""
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This load>
nvm install node
cd ./Application/frontend
npm install
echo ""
echo "====> Installing Application Backend Dependencies <===="
echo ""
cd ../backend
python3 -m venv venv
venv/bin/python -m pip install -r requirements.txt
