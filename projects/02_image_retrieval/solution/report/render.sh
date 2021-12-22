replace() {
  sed -i '' "s/$1/$2/" $3
}

# Metadata
title='\\title{Görüntü İşleme Ödev \\#2\\\\İçerik Tabanlı Görüntü Erişimi}'
author='\\author{Rameş Aliyev, 18011708\\\\l1118708@std.yildiz.edu.tr}'
wdate='\\date{Yıldız Teknik Üniversitesi — 2021 Güz}'

# Blocks
hline='\\textcolor[RGB]{170,170,170}{\\rule{\\linewidth}{0.1pt}}'

# Definitions
match_begin='\\maketitle'
define_color='\\definecolor{coolblack}{rgb}{0.0, 0.18, 0.39}'
define_blockcode='\\newenvironment{blockcode}{\\leavevmode\\small\\color{coolblack}\\verbatim}{\\endverbatim}'
replace_begin="$match_begin\n$define_color\n$define_blockcode"

# Partial Replacements
match_title='\\title{metatitle}'
replace_title=$title

match_section='\\section{Konu}'
replace_section='\\section*{Konu}'

match_authors='\\author{metaauthors}'
replace_authors="$author\n    $wdate"

match_code_start='\\begin{verbatim}'
match_code_end='\\end{verbatim}'
blockcode_start='\\vspace*{-12pt}\n\\begin{blockcode}'
blockcode_end='\\end{blockcode}\n\\vspace*{-10pt}'
replace_code_start="$hline\n$blockcode_start"
replace_code_end="$blockcode_end\n$hline"

texfile='report.tex'

# Flow

jupyter nbconvert --to latex --no-input report.ipynb

replace "$match_begin" "$replace_begin" "$texfile"
replace "$match_section" "$replace_section" "$texfile"
replace "$match_title" "$replace_title" "$texfile"
replace "$match_authors" "$replace_authors" "$texfile"
replace "$match_code_start" "$replace_code_start" "$texfile"
replace "$match_code_end" "$replace_code_end" "$texfile"

xelatex report.tex