<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>BowtieSettings</name>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="41"/>
        <source>Parameters</source>
        <translation type="unfinished">Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="74"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Maximum permitted total of quality values at all mismatched read positions throughout the entire alignment, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;not just in the &amp;quot;seed&amp;quot;. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;The default is 70. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Like Maq, bowtie rounds quality values to the nearest 10 and saturates at 30; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;rounding can be disabled with --nomaqround.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="85"/>
        <source>--maqerr</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="92"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;The &amp;quot;seed length&amp;quot;; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;i.e., the number of bases on the high-quality end of the read to which the -n ceiling applies.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;The lowest permitted setting is 5 and the default is 28. bowtie is faster for larger values of -l.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="101"/>
        <source>--seedlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="150"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;The maximum insert size for valid paired-end alignments. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;E.g. if -X 100 is specified and a paired-end alignment consists of two 20-bp alignments in the proper orientation with a 60-bp gap between them, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;that alignment is considered valid (as long as -I is also satisfied). &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;A 61-bp gap would not be valid in that case. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;If trimming options -3 or -5 are also used, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;the -X constraint is applied with respect to the untrimmed mates, not the trimmed mates.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Default: 250.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="163"/>
        <source>--maxbts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="228"/>
        <source>The number of megabytes of memory a given thread is given to store path descriptors in --best mode. 
Best-first search must keep track of many paths at once to ensure it is always extending the path with the lowest cumulative cost. 
Bowtie tries to minimize the memory impact of the descriptors, but they can still grow very large in some cases. 
If you receive an error message saying that chunk memory has been exhausted in --best mode, 
try adjusting this parameter up to dedicate more memory to the descriptors. 
Default: 64.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="236"/>
        <source>--chunkmbs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="253"/>
        <source>Use &lt;int&gt; as the seed for pseudo-random number generator</source>
        <translation type="unfinished">Использовать &lt;int&gt; в качестве основы для генератора псевдо-случайных чисел</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="256"/>
        <source>--seed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="314"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;When the -n option is specified (which is the default), &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;bowtie determines which alignments are valid according to the following policy, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;which is similar to Maq&apos;s default policy.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;In -v mode, alignments may have no more than V mismatches, where V may be a number from 0 through 3 set using the -v option. Quality values are ignored. The -v option is mutually exclusive with the -n option.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="329"/>
        <source>-n mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="334"/>
        <source>-v mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="349"/>
        <source>Flags</source>
        <translation type="unfinished">Флаги</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="363"/>
        <source>Using prebuilt index file instead of reference sequence</source>
        <translation type="unfinished">Использование предпостроенного индексного файла вместо референсной последовательности</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="366"/>
        <source>Prebuilt index</source>
        <translation type="unfinished">Предпостроенный индекс</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="373"/>
        <source>Sort result alignment by reads offset</source>
        <translation type="unfinished">Упорядочить выравнивание по смещениям считываемых последовательностей</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="376"/>
        <source>Sort alignment</source>
        <translation type="unfinished">Упорядочить выравнивание</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="386"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Maq accepts quality values in the Phred quality scale, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;but internally rounds values to the nearest 10, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;with a maximum of 30. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;By default, bowtie also rounds this way. --nomaqround prevents this rounding in bowtie.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="396"/>
        <source>--nomaqround</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="403"/>
        <source>If --nofw is specified, bowtie will not attempt to align against the forward reference strand.</source>
        <translation type="unfinished">Если --nofw указан, то bowtie не будет пытаться проводить сборку по прямой последовательности.</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="406"/>
        <source>--nofw</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="413"/>
        <source>If --norc is specified, bowtie will not attempt to align against the reverse-complement reference strand.</source>
        <translation type="unfinished">Если --norc указан, то bowtie не будет пытаться проводить сборку по обратно-комплементарной последовательности.</translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="416"/>
        <source>--norc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="423"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Try as hard as possible to find valid alignments when they exist, including paired-end alignments. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;This is equivalent to specifying very high values for the --maxbts and --pairtries options.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;This mode is generally much slower than the default settings, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;but can be useful for certain problems. &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;This mode is slower when (a) the reference is very repetitive, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;(b) the reads are low quality, &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;or (c) not many reads have valid alignments.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/BowtieSettings.ui" line="436"/>
        <source>--tryhard</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::BowtieAdapter</name>
    <message>
        <location filename="../src/BowtieAdapter.cpp" line="47"/>
        <source>Reference assembly failed - no possible alignment found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::BowtieBuildTask</name>
    <message>
        <location filename="../src/BowtieTask.cpp" line="67"/>
        <source>Reference index file &quot;%1&quot; not exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieTask.cpp" line="75"/>
        <source>Reference sequence file &quot;%1&quot; not exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieTask.cpp" line="183"/>
        <source>Bowtie Build</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieTask.cpp" line="188"/>
        <source>Reference file &quot;%1&quot; not exists</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::BowtiePlugin</name>
    <message>
        <location filename="../src/BowtiePlugin.cpp" line="42"/>
        <source>EBWT index</source>
        <translation type="unfinished">Индекс EBWT</translation>
    </message>
    <message>
        <location filename="../src/BowtiePlugin.cpp" line="42"/>
        <source>EBWT Index for bowtie</source>
        <translation type="unfinished">Индекс EBWT для bowtie</translation>
    </message>
    <message>
        <location filename="../src/BowtiePlugin.cpp" line="49"/>
        <source>Bowtie</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtiePlugin.cpp" line="50"/>
        <source>An ultrafast memory-efficient short read aligner, http://bowtie-bio.sourceforge.net</source>
        <translation type="unfinished">Быстрый и эффективно расходующий память выравниватель short read-ов, http://bowtie-bio.sourceforge.net</translation>
    </message>
</context>
<context>
    <name>U2::GTest_Bowtie</name>
    <message>
        <location filename="../src/bowtie_tests/bowtieTests.cpp" line="220"/>
        <source>Bowtie</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtieBuildPrompter</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="250"/>
        <source>Build ebwt index from %1 and send it url to output.</source>
        <translation type="unfinished">Построить индекс ebwt из %1 и послать его указатель на вывод.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtieBuildWorker</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="218"/>
        <source>EBWT index</source>
        <translation type="unfinished">Индекс EBWT</translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="218"/>
        <source>Result ebwt index.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="220"/>
        <source>Reference</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="221"/>
        <source>Reference sequence url. The short reads will be aligned to this reference genome.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="222"/>
        <source>Bowtie-build indexer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="224"/>
        <source>EBWT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="225"/>
        <source>Output index url.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="268"/>
        <location filename="../src/BowtieWorker.cpp" line="272"/>
        <source>Reference sequence URL is empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="292"/>
        <source>Bowtie index building finished. Result name is %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtieIndexReaderPrompter</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="333"/>
        <source>Read ebwt index from %1 and send it url to output.</source>
        <translation type="unfinished">Считать индекс EBWT из %1 и послать его указатель на вывод.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtieIndexReaderWorker</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="305"/>
        <source>EBWT index</source>
        <translation type="unfinished">Индекс EBWT</translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="305"/>
        <source>Result of alignment.</source>
        <translation type="unfinished">Результат выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="307"/>
        <source>Bowtie index reader</source>
        <translation type="unfinished">Считывание индекса Bowtie</translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="309"/>
        <source>EBWT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="310"/>
        <source>Output index url.</source>
        <translation type="unfinished">Выходной указатель на индекс.</translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="350"/>
        <source>Reference sequence URL is empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="363"/>
        <source>Reading bowtie index finished. Result name is %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtiePrompter</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="137"/>
        <location filename="../src/BowtieWorker.cpp" line="138"/>
        <source> from &lt;u&gt;%1&lt;/u&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="140"/>
        <source>Align short reads %1 to the reference genome %2 and send it to output.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::BowtieWorker</name>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="46"/>
        <source>Short read sequences</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="46"/>
        <source>Short reads to be aligned.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="47"/>
        <source>EBWT index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="47"/>
        <source>EBWT index of reference sequence.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="48"/>
        <source>Short reads alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="48"/>
        <source>Result of alignment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="56"/>
        <source>Bowtie aligner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="57"/>
        <source>An ultrafast memory-efficient short read aligner, http://bowtie-bio.sourceforge.net</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="60"/>
        <source>-n alignment mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="61"/>
        <source>Alignments may have no more than N mismatches (where N is a number 0-3, set with -n) in the first L bases 						 (where L is a number 5 or greater, set with -l) on the high-quality (left) end of the read. The first L bases are called the &quot;seed&quot;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="63"/>
        <source>-v alignment mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="64"/>
        <source>&quot;-1&quot; - use default value. Report alignments with at most &lt;int&gt; mismatches. -e and -l options are ignored and quality values have no effect on what alignments are valid. -v is mutually exclusive with -n.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="65"/>
        <source>Maximum permitted total of quality values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="66"/>
        <source>bowtie &quot;-e/--maqerr&quot; option. Maximum permitted total of quality values at all mismatched read positions throughout the entire alignment, not just in the &quot;seed&quot;. The default is 70.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="67"/>
        <source>Seed length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="68"/>
        <source>bowtie &quot;-l/--seedlen&quot; option. The &quot;seed length&quot;; i.e., the number of bases on the high-quality end of the read to which the -n ceiling applies. The lowest permitted setting is 5 and the default is 28. bowtie is faster for larger values of -l.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="69"/>
        <source>Skip Maq quality rounding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="70"/>
        <source>bowtie &quot;--nomaqround&quot; option. Maq accepts quality values in the Phred quality scale, but internally rounds values to the nearest 10, with a maximum of 30. By default, bowtie also rounds this way. --nomaqround prevents this rounding in bowtie.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="71"/>
        <source>Do not align against the forward reference strand</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="72"/>
        <source>bowtie &quot;--nofw&quot; option. If --nofw is specified, bowtie will not attempt to align against the forward reference strand.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="73"/>
        <source>Do not align against the reverse-complement reference strand</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="74"/>
        <source>bowtie &quot;--norc&quot; option. If --norc is specified, bowtie will not attempt to align against the reverse-complement reference strand.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="75"/>
        <source>Maximum number of backtracks permitted</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="76"/>
        <source>bowtie &quot;--maxbts&quot; option. The maximum number of backtracks permitted when aligning a read in -n 2 or -n 3 mode (default: 125 without --best, 800 with --best). 						 A &quot;backtrack&quot; is the introduction of a speculative substitution into the alignment. Without this limit, the default parameters will 						 sometimes require that bowtie try 100s or 1,000s of backtracks to align a read, especially if the read has many low-quality bases and/or 						 has no valid alignments, slowing bowtie down significantly. However, this limit may cause some valid alignments to be missed. Higher limits 						 yield greater sensitivity at the expensive of longer running times.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="81"/>
        <source>Tryhard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="82"/>
        <source>bowtie &quot;-y/--tryhard&quot; option.Try as hard as possible to find valid alignments when they exist, including paired-end alignments.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="83"/>
        <source>Chunk Mbs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="84"/>
        <source>bowtie &quot;--chunkmbs&quot; option. The number of megabytes of memory a given thread is given to store path descriptors in --best mode.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="85"/>
        <source>Pseudo random seed number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="86"/>
        <source>bowtie &quot;--seed&quot; option. Use &lt;int&gt; as the seed for pseudo-random number generator. &quot;-1&quot; - use pseudo random</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="173"/>
        <source>Loaded short read %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="180"/>
        <source>Short reads list is empty.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="185"/>
        <source>Reference sequence URL is empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="205"/>
        <source>Bowtie alignment finished. Result name is %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="223"/>
        <source>Bowtie-build builds a Bowtie index from a set of DNA sequences. bowtie-build outputs a set of 6 files with suffixes .1.ebwt, .2.ebwt, .3.ebwt, .4.ebwt, .rev.1.ebwt, and .rev.2.ebwt. These files together constitute the index: they are all that is needed to align reads to that reference. The original sequence files are no longer used by Bowtie once the index is built.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/BowtieWorker.cpp" line="308"/>
        <source>Read a set of 6 files with suffixes .1.ebwt, .2.ebwt, .3.ebwt, .4.ebwt, .rev.1.ebwt, and .rev.2.ebwt. These files together constitute the index: they are all that is needed to align reads to that reference.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
