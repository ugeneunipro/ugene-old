<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru">
<context>
    <name>PhyTreeGeneratorTask</name>
    <message>
        <location filename="../src/phyltree/PhyTreeGeneratorTask.cpp" line="10"/>
        <source>Calculating Phylogenetic Tree</source>
        <translation>Создание филогенетического дерева</translation>
    </message>
    <message>
        <location filename="../src/phyltree/PhyTreeGeneratorTask.cpp" line="32"/>
        <source>Calculating phylogenetic tree</source>
        <translation>Создание филогенетического дерева</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="37"/>
        <source>Loading OPENCL driver library</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="42"/>
        <source>Cannot load OpenCL library. Error while loading %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="49"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="56"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="63"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="70"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="79"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="86"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="93"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="100"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="107"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="114"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="121"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="128"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="135"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="142"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="149"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="156"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="163"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="170"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="177"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="184"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="191"/>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="198"/>
        <source>Cannot resolve symbol %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="214"/>
        <source>Cannot load library: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_gpu/opencl/OpenCLHelper.cpp" line="217"/>
        <source>Some errors occurs in library: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusUtils.cpp" line="88"/>
        <source>Gaps:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::FindAlgorithmTask</name>
    <message>
        <location filename="../src/misc/FindAlgorithmTask.cpp" line="10"/>
        <source>Find in sequence task</source>
        <translation>Поиск Смита-Ватермана</translation>
    </message>
</context>
<context>
    <name>U2::MSAConsensusAlgorithmFactoryClustal</name>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmClustal.cpp" line="9"/>
        <source>Emulates ClustalW program and file format behavior.</source>
        <translation>Эмулирует программу ClustalW и её интерпретацию формата файла.</translation>
    </message>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmClustal.cpp" line="13"/>
        <source>ClustalW</source>
        <translation>Схема ClustalW</translation>
    </message>
</context>
<context>
    <name>U2::MSAConsensusAlgorithmFactoryDefault</name>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmDefault.cpp" line="16"/>
        <source>Based on JalView algorithm. Returns &apos;+&apos; if there are 2 characters with high frequency. Returns symbol in lower case if the symbol content in a row is lower than the threshold specified.</source>
        <translation>В основе лежит алгоритм JalView. Возвращает &apos;+&apos;ˇ, если имеется 2 символа с высокой частотой. Возвращает символ в нижнем регистре, если символ содержится в ряду меньшее число раз, чем это указано в пороге.</translation>
    </message>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmDefault.cpp" line="20"/>
        <source>Default</source>
        <translation>Схема по умолчанию</translation>
    </message>
</context>
<context>
    <name>U2::MSAConsensusAlgorithmFactoryLevitsky</name>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmLevitsky.cpp" line="15"/>
        <source>The algorithm proposed by Victor Levitsky to work with DNA alignments.
Collects global alignment frequency for every symbol using extended (15 symbols) DNA alphabet first.
For every column selects the most rare symbol in the whole alignment with percentage in the column greater or equals to the threshold value.</source>
        <translation>Алгоритм представлен Виктором Левицким для работы с выравниваниями ДНК.
В первую очередь алгоритм вычисляет глобальную частоту появления каждого символа в выравнивании с использованием расширенного (15-символьного) алфавита ДНК.
Для каждого столбца выбирается наиболее редко встречаемый символ во всём выравнивании, такой что процентное соотношение его появления в столбце больше либо равно значению порога.</translation>
    </message>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmLevitsky.cpp" line="23"/>
        <source>Levitsky</source>
        <translation>Схема Левицкого</translation>
    </message>
</context>
<context>
    <name>U2::MSAConsensusAlgorithmFactoryStrict</name>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmStrict.cpp" line="18"/>
        <source>The algorithm returns gap character (&apos;-&apos;) if symbol frequency in a column is lower than threshold specified.</source>
        <translation>Алгоритм возвращает символ пропуска (&apos;-&apos;), если частота появления символа в столбце меньше, чем величина, заданная в пороге.</translation>
    </message>
    <message>
        <location filename="../src/util_msa_consensus/MSAConsensusAlgorithmStrict.cpp" line="22"/>
        <source>Strict</source>
        <translation>Строгая схема</translation>
    </message>
</context>
<context>
    <name>U2::MSADistanceAlgorithm</name>
    <message>
        <location filename="../src/util_msa_distance/MSADistanceAlgorithm.cpp" line="32"/>
        <source>MSA distance algorithm &quot;%1&quot; task</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::MSADistanceAlgorithmFactoryHamming</name>
    <message>
        <location filename="../src/util_msa_distance/MSADistanceAlgorithmHamming.cpp" line="15"/>
        <source>Based on Hamming distance between two sequences</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_msa_distance/MSADistanceAlgorithmHamming.cpp" line="19"/>
        <source>Hamming</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::MSADistanceAlgorithmFactoryHammingRevCompl</name>
    <message>
        <location filename="../src/util_msa_distance/MSADistanceAlgorithmHammingRevCompl.cpp" line="18"/>
        <source>Based on Hamming distance between two sequences</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_msa_distance/MSADistanceAlgorithmHammingRevCompl.cpp" line="22"/>
        <source>Hamming reverse-complement</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::MolecularSurfaceCalcTask</name>
    <message>
        <location filename="../src/molecular_geometry/MolecularSurface.cpp" line="102"/>
        <source>Molecular surface calculation</source>
        <translation>Вычисление молекулярной поверхности</translation>
    </message>
</context>
<context>
    <name>U2::ORFFindTask</name>
    <message>
        <location filename="../src/util_orf/ORFAlgorithmTask.cpp" line="10"/>
        <source>ORF find</source>
        <translation>Поиск ORF (рамок считывания)</translation>
    </message>
</context>
<context>
    <name>U2::PWMConversionAlgorithmFactoryBVH</name>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmBVH.cpp" line="21"/>
        <source>Berg and von Hippel weight function</source>
        <translation>Весовая функция Берга-Гиппеля</translation>
    </message>
</context>
<context>
    <name>U2::PWMConversionAlgorithmFactoryLOD</name>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmLOD.cpp" line="17"/>
        <source>log-odds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmLOD.cpp" line="21"/>
        <source>log-odds weight function</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::PWMConversionAlgorithmFactoryMCH</name>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmMCH.cpp" line="16"/>
        <source>Match</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmMCH.cpp" line="20"/>
        <source>Match weight function</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::PWMConversionAlgorithmFactoryNLG</name>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmNLG.cpp" line="16"/>
        <source>NLG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/util_weight_matrix/PWMConversionAlgorithmNLG.cpp" line="20"/>
        <source>NLG weight function</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::PhyTreeGeneratorTask</name>
    <message>
        <source>Calculating Phylogenetic Tree</source>
        <translation type="obsolete">Создание филогенетического дерева</translation>
    </message>
    <message>
        <source>Calculating phylogenetic tree</source>
        <translation type="obsolete">Создание филогенетического дерева</translation>
    </message>
</context>
<context>
    <name>U2::SecStructPredictTask</name>
    <message>
        <location filename="../src/registry/SecStructPredictTask.cpp" line="6"/>
        <source>Secondary structure predict</source>
        <translation>Предсказание вторичной структуры</translation>
    </message>
</context>
<context>
    <name>U2::SmithWatermanReportCallbackImpl</name>
    <message>
        <location filename="../src/smith_waterman/SmithWatermanReportCallback.cpp" line="18"/>
        <source>Annotation object not found.</source>
        <translation>Не найдена таблица аннотаций</translation>
    </message>
    <message>
        <location filename="../src/smith_waterman/SmithWatermanReportCallback.cpp" line="22"/>
        <source>Annotation table is read-only</source>
        <translation>Запрещена запись в таблицу аннотаций </translation>
    </message>
</context>
<context>
    <name>U2::SubstMatrixRegistry</name>
    <message>
        <source>Matrix %1 already registered.</source>
        <translation type="obsolete">Матрица %1 уже зарегестрирована.</translation>
    </message>
    <message>
        <source>Error parsing weight-matrix file: %1</source>
        <translation type="obsolete">Ошибка чтения матрицы: %1</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="70"/>
        <source>Reading substitution matrix from %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="78"/>
        <source>Error weight matrix file &apos;%1&apos; : %2</source>
        <translation>Ошибка в файле матрицы &quot;%1&quot; : &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="86"/>
        <source>Error opening file for read: %1</source>
        <translation>Ошибка открытия файла для чтения: &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="93"/>
        <source>Error reading file: %1</source>
        <translation>Ошибка чтения файла: &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="126"/>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="150"/>
        <source>Invalid character token &apos;%1&apos; , line %2</source>
        <translation>Неверный символ: &quot;%1&quot; в строке: &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="131"/>
        <source>Duplicate character &apos;%1&apos; , line %2</source>
        <translation>Дублированный символ: &quot;%1&quot; в строке &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="138"/>
        <source>Alphabet is neither nucleic nor protein!</source>
        <translation>Алфавит не является ни нуклеиновым, ни протеиновым!</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="142"/>
        <source>Matrix doesn&apos;t contains score for default alphabet character &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="155"/>
        <source>Invalid character row &apos;%1&apos; , line %2</source>
        <translation>Некорректная последовательность символов &quot;%1&quot; в строке: %2&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="159"/>
        <source>Duplicate character mapping &apos;%1&apos; , line %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="164"/>
        <source>Invalid number of columns &apos;%1&apos; , line %2</source>
        <translation>Неверное число столбцов &quot;%1&quot; в строке %2</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="172"/>
        <source>Can&apos;t parse numeric value &apos;%1&apos;, line %2</source>
        <translation>Не могу распознать числовое значение &quot;%1&quot; в строке &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="../src/registry/SubstMatrixRegistry.cpp" line="181"/>
        <source>Unexpected end of file!</source>
        <translation>Неожиданный конец файла!</translation>
    </message>
</context>
</TS>
