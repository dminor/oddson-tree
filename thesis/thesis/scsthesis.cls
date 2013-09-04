%% Carleton University, School of Computer Science MCS/PhD thesis class
%% file for LaTeX version 2e.
%%
%% Last edit Fri Jul 28 10:29:47 EDT 1995
%% Last edit by Murali Thiyagarajah

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{scsthesis}[1995/07/30 SCS Thesis Class]

\RequirePackage{ifthen}
\DeclareOption{mcs}{\gdef\degr@e{Master of Computer Science}}
\DeclareOption{phd}{\gdef\degr@e{Doctor of Philosophy}}
\ProcessOptions
\LoadClass[12pt]{report}


% We need 1" margins except on the binding edge, where it is 1 1/2"
% Theses are single-sided, so we don't care about \evensidemargin
\oddsidemargin 0.5in \evensidemargin 0in
\marginparwidth 40pt \marginparsep 10pt
\topmargin 0pt \headsep .5in
\textheight 8.1in \textwidth 6in

% Disallow page breaks at hyphens (this will give some underfull vbox's,
% so an alternative is to use \brokenpenalty=100 and manually search
% for and fix such page breaks)
\brokenpenalty=10000

% Use 1.3 times the normal baseline-to-baseline skip
\renewcommand{\baselinestretch}{1.3}

% Redefine the macro used for floats (including figures and tables)
% so that single spacing is used.
% (Note \def\figure{\@float{figure}set single spacing} doesn't work
%  because figure has an optional argument)
\def\@xfloat#1[#2]{\ifhmode \@bsphack\@floatpenalty -\@Mii\else
   \@floatpenalty-\@Miii\fi\def\@captype{#1}\ifinner
      \@parmoderr\@floatpenalty\z@
    \else\@next\@currbox\@freelist{\@tempcnta\csname ftype@#1\endcsname
       \multiply\@tempcnta\@xxxii\advance\@tempcnta\sixt@@n
       \@tfor \@tempa :=#2\do
                        {\if\@tempa h\advance\@tempcnta \@ne\fi
                         \if\@tempa t\advance\@tempcnta \tw@\fi
                         \if\@tempa b\advance\@tempcnta 4\relax\fi
                         \if\@tempa p\advance\@tempcnta 8\relax\fi
         }\global\count\@currbox\@tempcnta}\@fltovf\fi
    \global\setbox\@currbox\vbox\bgroup 
    \def\baselinestretch{1}\@normalsize
    \boxmaxdepth\z@
    \hsize\columnwidth \@parboxrestore}

% Redefine the macro used for footnotes to use single spacing
\long\def\@footnotetext#1{\insert\footins{\def\baselinestretch{1}\footnotesize
    \interlinepenalty\interfootnotelinepenalty 
    \splittopskip\footnotesep
    \splitmaxdepth \dp\strutbox \floatingpenalty \@MM
    \hsize\columnwidth \@parboxrestore
   \edef\@currentlabel{\csname p@footnote\endcsname\@thefnmark}\@makefntext
    {\rule{\z@}{\footnotesep}\ignorespaces
      #1\strut}}}

% \author, \title are defined in report; here are the rest of the
% front matter defining macros
\def\dept#1{\gdef\@dept{#1}}
\def\thesissupervisor#1{\gdef\@thesissupervisor{#1}}
\def\advis@r{Adviser} % default spelling
\def\thesissupervisor#1{\gdef\@thesissupervisor{#1}\gdef\advis@r{Advisor}}
\def\director#1{\gdef\@director{#1}}
\def\externalexaminer#1{\gdef\@externalexaminer{#1}}
\def\submitdate#1{\gdef\@submitdate{#1}}
\def\copyrightyear#1{\gdef\@copyrightyear{#1}} % \author, \title in report
\def\@title{}\def\@author{}\def\@dept{computer science}
\def\@thesissupervisor{}\def\@director{}\def\@externalexaminer{}
\def\@submitdate{\ifcase\the\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space \number\the\year}
  \def\@copyrightyear{\number\the\year}
\newif\ifcopyright \newif\iffigurespage \newif\iftablespage
\copyrighttrue \figurespagetrue \tablespagetrue


\def\titlep{%
	\thispagestyle{empty}%
	\null\vskip.25in%
	\begin{center}
	{\bf	\LARGE\expandafter{\@title}}
	\end{center}
	\vfill
        \begin{center}
		   \large{By\\
		 \@author}\\
	\end{center}\vskip.5in
	\begin{center}
		 A thesis submitted to\\
		the Faculty of Graduate Studies and Research\\
		in partial fulfilment of \\
		the requirements for the degree of\\
		\degr@e\\
	\end{center}
	\vfill
	\begin{center}
		Ottawa-Carleton Institute for Computer Science\\
		   School of Computer Science\\
			Carleton University\\
			Ottawa, Ontario\\
			\vskip.25in
		       \@submitdate\\
			\vskip.25in
			\copyright\ Copyright\\
			\@copyrightyear, \@author
	\end{center}\vskip.5in\newpage}

\long\def\signature#1{%
\begin{center}
\parindent=0pt
\vskip1in
The undersigned hereby recommend to\\
 the Faculty of
Graduate Studies and Research\\
 aceeptance of the thesis,\\
\vskip0.25in
\begin{center}
{\large
\@title\\}
\vskip.25in
submitted by\\
\vskip.25in
{\large
\@author}\\
\par
\vspace{.5in}
\vskip.25in
\shortstack{\vrule width 3in height 0.4pt\\#1}
 \vspace{.5in}
 \vskip.5in
        \shortstack{\vrule width 3in height 0.4pt\\
                        \@thesissupervisor\\
                        (Thesis Supervisor)}
        \vspace{.5in}
	\vskip.5in
	\ifthenelse{\equal{\degr@e}{Doctor of Philosophy}}{
        \shortstack{\vrule width 3in height 0.4pt\\
                        \@externalexaminer\\
                        (External Examiner)}}{}
\end{center}
\end{center}}

\def\signaturepage{%
	\vskip0.75in
	\signature{\@director\\(Director, School of Computer Science)}
	\vfill
	\vfill
	\begin{center}
		Carleton University\\
			\@submitdate
	\end{center}}

\def\beforepreface{
	\pagenumbering{roman}
	\pagestyle{plain}
	\titlep
	\signaturepage}

\def\prefacesection#1{%
	\chapter*{#1}
	\addcontentsline{toc}{chapter}{#1}}

\def\afterpreface{\newpage
	\tableofcontents
	\newpage
	%\iftablespage
	%	\listoftables
	%	\newpage
	%\fi
	\iffigurespage
		\listoffigures
		\newpage
	\fi
	\pagenumbering{arabic}
	\pagestyle{headings}}

% Redefine \thebibliography to go to a new page and put an entry in the
% table of contents
\let\@ldthebibliography\thebibliography
\renewcommand{\thebibliography}[1]{\newpage
	\addcontentsline{toc}{chapter}{Bibliography}
	\@ldthebibliography{#1}}

% Start out normal
\pagestyle{headings}

