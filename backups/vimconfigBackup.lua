-- ========= BASIC =========
vim.o.number = true
vim.o.relativenumber = true
vim.o.expandtab = true
vim.o.shiftwidth = 4
vim.o.tabstop = 4
vim.o.clipboard = "unnamedplus"
vim.g.mapleader = " "

vim.opt.completeopt = { "menu", "menuone", "noselect" }

-- ========= LAZY BOOTSTRAP =========
local lazypath = vim.fn.stdpath("data") .. "/lazy/lazy.nvim"

if not vim.loop.fs_stat(lazypath) then
    vim.fn.system({
        "git",
        "clone",
        "--filter=blob:none",
        "https://github.com/folke/lazy.nvim.git",
        "--branch=stable",
        lazypath,
    })
end

vim.opt.rtp:prepend(lazypath)

-- ========= PLUGINS =========
require("lazy").setup({

    { "neovim/nvim-lspconfig" },

    {
        "hrsh7th/nvim-cmp",
        dependencies = {
            "hrsh7th/cmp-nvim-lsp",
        },
        config = function()
            local cmp = require("cmp")

            cmp.setup({
                completion = {
                    autocomplete = false,
                    completeopt = "menu,menuone,noinsert",
                },

                performance = {
                    max_view_entries = 5,
                    debounce = 120,
                    throttle = 60,
                    fetching_timeout = 150,
                },

                preselect = cmp.PreselectMode.None,

                mapping = cmp.mapping.preset.insert({
                    ["<C-Space>"] = cmp.mapping.complete(),
                    ["<CR>"] = cmp.mapping.confirm({ select = false }),
                    ["<Tab>"] = cmp.mapping.select_next_item(),
                    ["<S-Tab>"] = cmp.mapping.select_prev_item(),
                }),

                sources = {
                    { name = "nvim_lsp", max_item_count = 5 },
                    { name = "buffer", max_item_count = 5 },
                },

                experimental = {
                    ghost_text = false,
                },
            })

            -- Completion bewusst via Leader
            vim.keymap.set("n", "<leader>cc", function()
                vim.cmd("startinsert")
                cmp.complete()
            end)

            vim.keymap.set("i", "<leader>cc", function()
                cmp.complete()
            end)
        end,
    },

    {
        "catppuccin/nvim",
        name = "catppuccin",
        priority = 1000,
        config = function()
            require("catppuccin").setup({
                flavour = "macchiato",
            })
            vim.cmd.colorscheme("catppuccin")
        end,
    },

    {
        "nvim-telescope/telescope.nvim",
        dependencies = { "nvim-lua/plenary.nvim" },
    },

})

-- ========= LSP =========

local capabilities = require("cmp_nvim_lsp").default_capabilities()

vim.lsp.config("clangd", {
    cmd = {
        "clangd",
        "--background-index=false",
        "--clang-tidy=false",
        "--all-scopes-completion=false",
        "--limit-results=50",
        "--threads=2",
    },
    capabilities = capabilities,
})

vim.lsp.enable("clangd")

-- ========= DIAGNOSTICS =========

vim.diagnostic.config({
    update_in_insert = false,  -- KEINE Diagnostics während Tippen
    virtual_text = true,
    underline = true,
    severity_sort = true,
})

-- ========= LSP Keymaps =========

local telescope = require("telescope.builtin")

vim.keymap.set("n", "gd", vim.lsp.buf.definition)
vim.keymap.set("n", "K", vim.lsp.buf.hover)
vim.keymap.set("n", "gr", vim.lsp.buf.references)

vim.keymap.set("n", "<leader>cr", vim.lsp.buf.rename)
vim.keymap.set("n", "<leader>ca", vim.lsp.buf.code_action)
vim.keymap.set("n", "<leader>cf", function()
    vim.lsp.buf.format({ async = false })
end)

vim.keymap.set("n", "<leader>de", vim.diagnostic.open_float)
vim.keymap.set("n", "<leader>dq", vim.diagnostic.setloclist)

vim.keymap.set("n", "[d", vim.diagnostic.goto_prev)
vim.keymap.set("n", "]d", vim.diagnostic.goto_next)

-- ========= Telescope =========

vim.keymap.set("n", "<leader>ff", telescope.find_files)
vim.keymap.set("n", "<leader>fg", telescope.live_grep)
vim.keymap.set("n", "<leader>fb", telescope.buffers)

-- ========= Windows =========

vim.keymap.set("n", "<leader>wh", "<C-w>h")
vim.keymap.set("n", "<leader>wj", "<C-w>j")
vim.keymap.set("n", "<leader>wk", "<C-w>k")
vim.keymap.set("n", "<leader>wl", "<C-w>l")
vim.keymap.set("n", "<leader>wv", "<C-w>v")
vim.keymap.set("n", "<leader>ws", "<C-w>s")
vim.keymap.set("n", "<leader>wq", "<C-w>q")
