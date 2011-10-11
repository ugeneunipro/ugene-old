<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="en_US">
<context>
    <name>ORFDialogBase</name>
    <message>
        <source>ORF Marker</source>
        <translation></translation>
    </message>
    <message>
        <source>Strand</source>
        <translation></translation>
    </message>
    <message>
        <source>Both</source>
        <translation></translation>
    </message>
    <message>
        <source>Search in both strands</source>
        <translation></translation>
    </message>
    <message>
        <source>Direct</source>
        <translation></translation>
    </message>
    <message>
        <source>Search frames in direct strand only</source>
        <translation></translation>
    </message>
    <message>
        <source>Complement</source>
        <translation></translation>
    </message>
    <message>
        <source>Search frames in complement strand only</source>
        <translation></translation>
    </message>
    <message>
        <source>Search Settings</source>
        <translation></translation>
    </message>
    <message>
        <source>Min length, bp:</source>
        <translation></translation>
    </message>
    <message>
        <source>Ignore ORFs shorter than the specified length</source>
        <translation></translation>
    </message>
    <message>
        <source>Must terminate within region</source>
        <translation></translation>
    </message>
    <message>
        <source>Must start with init codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Specifies that each ORF found must start with the start codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Allow overlaps</source>
        <translation></translation>
    </message>
    <message>
        <source>Allow alternative init codons</source>
        <translation></translation>
    </message>
    <message>
        <source>Include stop codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Preview</source>
        <translation></translation>
    </message>
    <message>
        <source>Clear results</source>
        <translation></translation>
    </message>
    <message>
        <source>Region</source>
        <translation></translation>
    </message>
    <message>
        <source>Length</source>
        <translation></translation>
    </message>
    <message>
        <source>Find Open Reading Frames in sequence</source>
        <translation></translation>
    </message>
    <message>
        <source>Start searching ORFs</source>
        <translation></translation>
    </message>
    <message>
        <source>
               Allow alternative (downstream) initiators, when another start codon is located within a longer ORF.
               Then all possible ORF will be found, not only the longest ones.
             </source>
        <translation></translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt;&quot;&gt;Include stop codon into resulting annotation. &lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>
               Allow ORFs starting with alternative initiation codons,
               accordingly to the current translation table.
             </source>
        <translation></translation>
    </message>
    <message>
        <source>Ignore boundary ORFs which last beyond the search region(i.e. have no stop codon within the range).
             </source>
        <translation></translation>
    </message>
    <message>
        <source>
               Allow ORFs starting with alternative initiation codons,
               accordingly to the current translation table.
</source>
        <translation></translation>
    </message>
    <message>
        <source>Require stop codon</source>
        <translation></translation>
    </message>
    <message>
        <source>
Ignore boundary ORFs which last beyond the search region
(i.e. have no stop codon within the range).
</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::FindORFsToAnnotationsTask</name>
    <message>
        <source>Find ORFs and save to annotations</source>
        <translation></translation>
    </message>
    <message>
        <source>Annotation obj %1 is locked for modifications</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::ORFPrompter</name>
    <message>
        <source>unset</source>
        <translation></translation>
    </message>
    <message>
        <source> from &lt;u&gt;%1&lt;/u&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>starting with any codon</source>
        <translation></translation>
    </message>
    <message>
        <source>, allow ORFs %1 other than terminator</source>
        <translation></translation>
    </message>
    <message>
        <source>, take into account %1</source>
        <translation></translation>
    </message>
    <message>
        <source>ignore non-terminated</source>
        <translation></translation>
    </message>
    <message>
        <source>, %1 ORFs</source>
        <translation></translation>
    </message>
    <message>
        <source>For each nucleotide sequence%1, find ORFs in &lt;u&gt;%2&lt;/u&gt; using the &lt;u&gt;%3&lt;/u&gt;.&lt;br&gt;Detect only ORFs &lt;u&gt;not shorter than %4 bps&lt;/u&gt;%5.&lt;br&gt;Output the list of found regions annotated as &lt;u&gt;%6&lt;/u&gt;.</source>
        <translation></translation>
    </message>
    <message>
        <source>alternative start codons</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::ORFWorker</name>
    <message>
        <source>Input sequences</source>
        <translation></translation>
    </message>
    <message>
        <source>A nucleotide sequence to search ORFs in. Protein sequences are skipped.</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF annotations</source>
        <translation></translation>
    </message>
    <message>
        <source>A set of annotations marking ORFs found in the sequence.</source>
        <translation></translation>
    </message>
    <message>
        <source>Annotate as</source>
        <translation></translation>
    </message>
    <message>
        <source>Name of the result annotations marking found ORFs</source>
        <translation></translation>
    </message>
    <message>
        <source>Genetic code</source>
        <translation></translation>
    </message>
    <message>
        <source>Which genetic code should be used for translating the input nucleotide sequence.</source>
        <translation></translation>
    </message>
    <message>
        <source>Require stop codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Require init codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Allow alternative codons</source>
        <translation></translation>
    </message>
    <message>
        <source>The result annotation will includes stop codon if this option is set</source>
        <translation></translation>
    </message>
    <message>
        <source>Finds Open Reading Frames (ORFs) in each supplied nucleotide sequence, stores found regions as annotations.&lt;p&gt;Protein sequences are skipped if any supplied to input.&lt;p&gt;&lt;dfn&gt;ORFs are DNA sequence regions that could potentially encode a protein, and usually give a good indication of the presence of a gene in the surrounding sequence.&lt;/dfn&gt;&lt;/p&gt;&lt;p&gt;In the sequence, ORFs are located between a start-code sequence (initiation codon) and a stop-code sequence (termination codon), defined by the selected genetic code.&lt;/p&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>both strands</source>
        <translation></translation>
    </message>
    <message>
        <source>direct strand</source>
        <translation></translation>
    </message>
    <message>
        <source>complement strand</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF: result name is empty, default name used</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF: Incorrect value: min-length must be greater then zero</source>
        <translation></translation>
    </message>
    <message>
        <source>Incorrect value: min-length must be greater then zero</source>
        <translation></translation>
    </message>
    <message>
        <source>Bad sequence supplied to ORFWorker: %1</source>
        <translation></translation>
    </message>
    <message>
        <source>Found %1 ORFs</source>
        <translation></translation>
    </message>
    <message>
        <source>Min length, bp:</source>
        <translation></translation>
    </message>
    <message>
        <source>Include stop codon</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF Marker</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::ORFAutoAnnotationsUpdater</name>
    <message>
        <source>ORFs</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::ORFDialog</name>
    <message>
        <source>%1 results found.</source>
        <translation></translation>
    </message>
    <message>
        <source>Complement</source>
        <translation></translation>
    </message>
    <message>
        <source>Direct</source>
        <translation></translation>
    </message>
    <message>
        <source>Start codons</source>
        <translation>Start codons:</translation>
    </message>
    <message>
        <source>Alternative start codons</source>
        <translation>Alternative start codons:</translation>
    </message>
    <message>
        <source>Stop codons</source>
        <translation>Stop codons:</translation>
    </message>
    <message>
        <source>Results list contains results from the previous search. Clear?</source>
        <translation></translation>
    </message>
    <message>
        <source>Progress %1%</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::ORFMarkerPlugin</name>
    <message>
        <source>ORF Marker</source>
        <translation></translation>
    </message>
    <message>
        <source>Searches for open reading frames (ORF) in a DNA sequence.</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::ORFViewContext</name>
    <message>
        <source>Find ORFs...</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::QDORFActor</name>
    <message>
        <source>both strands</source>
        <translation></translation>
    </message>
    <message>
        <source>direct strand</source>
        <translation></translation>
    </message>
    <message>
        <source>complement strand</source>
        <translation></translation>
    </message>
    <message>
        <source>, allow ORFs &lt;u&gt;starting with any codon&lt;/u&gt; other than terminator</source>
        <translation></translation>
    </message>
    <message>
        <source>, take into account &lt;u&gt;alternative start codons&lt;/u&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>, &lt;u&gt;ignore non-terminated&lt;/u&gt; ORFs</source>
        <translation></translation>
    </message>
    <message>
        <source>Finds ORFs in &lt;u&gt;%1&lt;/u&gt; using the &lt;u&gt;%2&lt;/u&gt;.&lt;br&gt;Detects only ORFs &lt;u&gt;not shorter than %3, not longer than %4&lt;/u&gt;%5.</source>
        <translation></translation>
    </message>
    <message>
        <source>Bad sequence</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF find</source>
        <translation></translation>
    </message>
    <message>
        <source>Finds Open Reading Frames (ORFs) in supplied nucleotide sequence, stores found regions as annotations.&lt;p&gt;Protein sequences are skipped if any.&lt;p&gt;&lt;dfn&gt;ORFs are DNA sequence regions that could potentially encode a protein, and usually give a good indication of the presence of a gene in the surrounding sequence.&lt;/dfn&gt;&lt;/p&gt;&lt;p&gt;In the sequence, ORFs are located between a start-code sequence (initiation codon) and a stop-code sequence (termination codon), defined by the selected genetic code.&lt;/p&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>Max length</source>
        <translation></translation>
    </message>
    <message>
        <source>Maximum length of annotation allowed.</source>
        <translation></translation>
    </message>
    <message>
        <source>ORF</source>
        <translation></translation>
    </message>
    <message>
        <source>Genetic code</source>
        <translation></translation>
    </message>
    <message>
        <source>Min length, bp:</source>
        <translation></translation>
    </message>
    <message>
        <source>Require init codon</source>
        <translation></translation>
    </message>
    <message>
        <source>Allow alternative codons</source>
        <translation></translation>
    </message>
    <message>
        <source>Which genetic code should be used for translating the input nucleotide sequence.</source>
        <translation></translation>
    </message>
    <message>
        <source>Require stop codon</source>
        <translation></translation>
    </message>
</context>
</TS>
