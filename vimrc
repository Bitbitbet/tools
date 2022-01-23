highlight pmenu ctermbg=189 ctermfg=0

" Enable to move a line in normal mode
noremap <C-k> ddkP
noremap <C-j> ddp
" inoremap ( ()
" inoremap [ []
" inoremap { {}

set mouse=a
set scrolloff=5
set number
set relativenumber
set hlsearch
set ignorecase
set smartcase

map H 7h
map L 7l
map J 5j
map K 5k

noremap <silent> <C-n> :nohlsearch<CR>
noremap <silent> tn :tabnew<CR>
noremap <silent> th :tabprev<CR>
noremap <silent> tl :tabnext<CR>

noremap <silent> sh :set nosplitright<CR>:vsplit<CR>
noremap <silent> sl :set splitright<CR>:vsplit<CR>
noremap <silent> sk :set nosplitbelow<CR>:split<CR>
noremap <silent> sj :set splitbelow<CR>:split<CR>

noremap <silent> <LEADER>h <C-w>h
noremap <silent> <LEADER>j <C-w>j
noremap <silent> <LEADER>k <C-w>k
noremap <silent> <LEADER>l <C-w>l

exec "nohlsearch"

au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif

syntax on
