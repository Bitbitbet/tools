" --------------vimspector settings--------------
map <F3> <Plug>VimspectorStop
map <F4> <Plug>VimspectorRestart
map <F5> <Plug>VimspectorContinue
map <F6> <Plug>VimspectorPause
map <F7> <Plug>VimspectorBalloonEval
map <F8> <Plug>VimspectorAddFunctionBreakpoint
map <F9> <Plug>VimspectorToggleBreakpoint
map <LEADER><F9> <Plug>VimspectorToggleConditionalBreakpoint
map <F10> <Plug>VimspectorStepOver
map <LEADER><F10> <Plug>VimspectorStepInto
map <F12> <Plug>VimspectorStepout
map <LEADER>r <Plug>VimspectorRunToCursor
" F3: stop
" F4: restart
" F5: start or continue
" F6: pause
" F7: Balloon Eval
" F8 F9 <LEADER>F9: Breakpoint 
" F10: Step over
" <LEADER>F10: Step into
" F12: Step out of

map <silent> <LEADER><F3> :VimspectorReset<CR>


" --------------loading plugins--------------
call plug#begin('~/.config/nvim/plug')

Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'vim-airline/vim-airline'
Plug 'vim-airline/vim-airline-themes'
Plug 'nvim-treesitter/nvim-treesitter', {'do': ':TSUpdate'}
Plug 'iamcco/markdown-preview.nvim', { 'do': 'cd app && yarn install'  }
Plug 'ajmwagar/vim-deus'
Plug 'puremourning/vimspector'
" Plug 'Yggdroot/indentLine'
Plug 'mg979/vim-visual-multi', {'branch': 'master'}

call plug#end()



" -----------visual-cursor settings-----------
let g:VM_maps = {}
let g:VM_maps['Find Under']         = '<C-d>'
let g:VM_maps['Find Subword Under'] = '<C-d>'



" --------------airline settings--------------
let g:airline_powerline_fonts=1
let g:airline#extensions#tabline#enabled = 1
let g:airline_theme='light'
let g:airline#extensions#coc#enabled = 1



" --------------coc.nvim settings--------------
let g:coc_global_extensions = ['coc-json', 'coc-vimlsp', 'coc-clangd', 'coc-jedi', 'coc-explorer', 'coc-html']

" completion configuration
noremap <silent><expr> <C-space> coc#refresh()
inoremap <silent><expr> <C-space> coc#refresh()

inoremap <silent><expr> <TAB>
	\ pumvisible() ? "\<C-n>" :
	\ <SID>check_back_space() ? "\<TAB>" :
	\ coc#refresh()
inoremap <silent><expr> <cr> pumvisible() ? coc#_select_confirm()
				\: "\<C-g>u\<CR>\<c-r>=coc#on_enter()\<CR>"
inoremap <expr><S-TAB> pumvisible() ? "\<C-p>" : "\<C-h>"
function! s:check_back_space() abort
  let col = col('.') - 1
  return !col || getline('.')[col - 1]  =~# '\s'
endfunction

nmap <silent> g[ <Plug>(coc-diagnostic-prev)
nmap <silent> g] <Plug>(coc-diagnostic-next)
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gy <Plug>(coc-type-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)

" highlight the symbol and its references when holding the cursor
autocmd CursorHold * silent call CocActionAsync('highlight')

" symbol renaming
nmap <leader>rn <Plug>(coc-rename)

" code action
xmap <leader>a  <Plug>(coc-codeaction-selected)
nmap <leader>a  <Plug>(coc-codeaction-selected)

" code formating
xmap <leader>f  <Plug>(coc-format-selected)
nmap <leader>f  <Plug>(coc-format-selected)

" scroll in popups
nnoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? coc#float#scroll(1) : "\<C-f>"
nnoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? coc#float#scroll(0) : "\<C-b>"
inoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? "\<c-r>=coc#float#scroll(1)\<cr>" : "\<Right>"
inoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? "\<c-r>=coc#float#scroll(0)\<cr>" : "\<Left>"
vnoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? coc#float#scroll(1) : "\<C-f>"
vnoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? coc#float#scroll(0) : "\<C-b>"

" documentation preview
nnoremap <silent> <LEADER>d :call <SID>show_documentation()<CR>
function! s:show_documentation()
  if (index(['vim','help'], &filetype) >= 0)
    execute 'h '.expand('<cword>')
  elseif (coc#rpc#ready())
    call CocActionAsync('doHover')
  else
    execute '!' . &keywordprg . " " . expand('<cword>')
  endif
endfunction

" shortcut for coc#float#jump()
nnoremap <silent> <LEADER>p :call coc#float#jump()<CR>

" better selection
xmap if <Plug>(coc-funcobj-i)
omap if <Plug>(coc-funcobj-i)
xmap af <Plug>(coc-funcobj-a)
omap af <Plug>(coc-funcobj-a)
xmap ic <Plug>(coc-classobj-i)
omap ic <Plug>(coc-classobj-i)
xmap ac <Plug>(coc-classobj-a)
omap ac <Plug>(coc-classobj-a)
nmap <silent> <C-s> <Plug>(coc-range-select)
xmap <silent> <C-s> <Plug>(coc-range-select)

set pumheight=15
set pumwidth=20
set shortmess+=c

" coc-explorer
nnoremap <silent> <LEADER>e :CocCommand explorer<CR>



" --------------nvim-treesitter settings--------------
lua<<EOF
require'nvim-treesitter.configs'.setup {
	ensure_installed = {"c", "cpp", "vim", "java", "python", "html", "json", "javascript", "bash", "lua"},
	highlight = {
		enable = true
	},
	indent = {
		enable = false
	}
}
EOF



" --------------markdown-preview.nvim settings--------------
" set to 1, nvim will open the preview window after entering the markdown buffer
" let g:mkdp_auto_start = 0

" set to 1, the nvim will auto close current preview window when change
" from markdown buffer to another buffer
" let g:mkdp_auto_close = 1

" set to 1, the vim will refresh markdown when save the buffer or
" leave from insert mode, default 0 is auto refresh markdown as you edit or
" move the cursor
" let g:mkdp_refresh_slow = 0

" set to 1, the MarkdownPreview command can be use for all files,
" by default it can be use in markdown file
" let g:mkdp_command_for_global = 0

" set to 1, preview server available to others in your network
" by default, the server listens on localhost (127.0.0.1)
let g:mkdp_open_to_the_world = 1

" use custom IP to open preview page
" useful when you work in remote vim and preview on local browser
" more detail see: https://github.com/iamcco/markdown-preview.nvim/pull/9
" let g:mkdp_open_ip = ''

" specify browser to open preview page
" let g:mkdp_browser = ''

" set to 1, echo preview page url in command line when open preview page
let g:mkdp_echo_preview_url = 1

" a custom vim function name to open preview page
" this function will receive url as param
" let g:mkdp_browserfunc = ''

" options for markdown render
" mkit: markdown-it options for render
" katex: katex options for math
" uml: markdown-it-plantuml options
" maid: mermaid options
" disable_sync_scroll: if disable sync scroll, default 0
" sync_scroll_type: 'middle', 'top' or 'relative', default value is 'middle'
"   middle: mean the cursor position alway show at the middle of the preview page
"   top: mean the vim top viewport alway show at the top of the preview page
"   relative: mean the cursor position alway show at the relative positon of the preview page
" hide_yaml_meta: if hide yaml metadata, default is 1
" sequence_diagrams: js-sequence-diagrams options
" content_editable: if enable content editable for preview page, default: v:false
" disable_filename: if disable filename header for preview page, default: 0
" let g:mkdp_preview_options = {
"     \ 'mkit': {},
"     \ 'katex': {},
"     \ 'uml': {},
"     \ 'maid': {},
"     \ 'disable_sync_scroll': 0,
"     \ 'sync_scroll_type': 'middle',
"     \ 'hide_yaml_meta': 1,
"     \ 'sequence_diagrams': {},
"     \ 'flowchart_diagrams': {},
"     \ 'content_editable': v:false,
"     \ 'disable_filename': 0
"     \ }

" use a custom markdown style must be absolute path
" like '/Users/username/markdown.css' or expand('~/markdown.css')
" let g:mkdp_markdown_css = ''

" use a custom highlight style must absolute path
" like '/Users/username/highlight.css' or expand('~/highlight.css')
" let g:mkdp_highlight_css = ''

" use a custom port to start server or random for empty
" let g:mkdp_port = ''

" preview page title
" ${name} will be replace with the file name
" let g:mkdp_page_title = '「${name}」'

" recognized filetypes
" these filetypes will have MarkdownPreview... commands
" let g:mkdp_filetypes = ['markdown']



" --------------deus settings--------------
colorscheme deus
highlight Normal ctermbg=none


" --------------nvim settings--------------
" Enable to move a line in normal mode
noremap <silent> <C-k> kddpk
noremap <silent> <C-j> ddp
" inoremap ( ()
" inoremap [ []
" inoremap { {}

set mouse=a
set scrolloff=5
set number
set relativenumber
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

" filetype indent off
" set nocindent
" set nosmartindent
" set noautoindent


exec "nohlsearch"
au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif



exe 'source ~/.config/nvim/custom.vim'
