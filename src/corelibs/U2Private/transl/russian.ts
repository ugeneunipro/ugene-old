<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru">
<context>
    <name>U2::AddPluginTask</name>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="336"/>
        <source>Add plugin task: %1</source>
        <translation>Загрузка внешнего модуля: %1</translation>
    </message>
    <message>
        <source>plugin_file_not_found_%1</source>
        <translation type="obsolete">Не найден внешний модуль: %1</translation>
    </message>
    <message>
        <source>plugin_invalid_file_format_%1</source>
        <translation type="obsolete">Некорректный файл библиотеки: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="343"/>
        <source>Plugin is already loaded: %1</source>
        <translation>Модуль уже загружен: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="351"/>
        <source>Plugin %1 depends on %2 which is not loaded</source>
        <translation>Модуль %1 зависит от модуля %2, который не был загружен</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="355"/>
        <source>Plugin %1 depends on %2 which is available, but the version is too old</source>
        <translation>Модуль %1 зависит от модуля %2, который доступен, но версия которого устарела</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="366"/>
        <source>Plugin loading error: %1, Error string %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin loading error: %1</source>
        <translation type="obsolete">Ошибка инициализации модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="373"/>
        <source>Plugin initialization routine was not found: %1</source>
        <translation>Не найдена входная функция библиотеки модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="379"/>
        <source>Plugin initialization failed: %1</source>
        <translation>Ошибка инициализации модуля: %1</translation>
    </message>
</context>
<context>
    <name>U2::DisableServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="260"/>
        <source>Disable &apos;%1&apos; service</source>
        <translation>Остановка сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="273"/>
        <source>Active top-level task name: %1</source>
        <translation>Активная задача: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="275"/>
        <source>Active task was found</source>
        <translation>Имеются неоконченные задачи</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="279"/>
        <source>Service is not registered: %1</source>
        <translation>Сервис не зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="283"/>
        <source>Service is not enabled: %1</source>
        <translation>Сервис не запущен: %1</translation>
    </message>
</context>
<context>
    <name>U2::EnableServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="160"/>
        <source>Service is already enabled: %1</source>
        <translation>Сервис запущен: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="166"/>
        <source>Circullar service dependency: %1</source>
        <translation>Циркулярная зависимость модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="172"/>
        <source>Required service is not enabled: %1</source>
        <translation>Не запущен обеспечивающий зависимость сервис: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="147"/>
        <source>Enable &apos;%1&apos; service</source>
        <translation>Запуск сервиса: %1</translation>
    </message>
</context>
<context>
    <name>U2::LoadAllPluginsTask</name>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="64"/>
        <source>Loading start up plugins</source>
        <translation>Загрузка подключаемых модулей</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="94"/>
        <source>File not found: %1</source>
        <translation>Не найден внешний модуль: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="99"/>
        <source>Invalid file format: %1</source>
        <translation>Некорректный файл библиотеки: %1</translation>
    </message>
</context>
<context>
    <name>U2::LogCategories</name>
    <message>
        <location filename="../src/LogSettings.cpp" line="15"/>
        <source>TRACE</source>
        <translation>Трассировка</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="16"/>
        <source>DETAILS</source>
        <translation>Детали</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="17"/>
        <source>INFO</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="18"/>
        <source>ERROR</source>
        <translation>Ошибки</translation>
    </message>
</context>
<context>
    <name>U2::PluginDescriptorHelper</name>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="88"/>
        <location filename="../src/PluginDescriptor.cpp" line="94"/>
        <location filename="../src/PluginDescriptor.cpp" line="100"/>
        <location filename="../src/PluginDescriptor.cpp" line="106"/>
        <source>Required attribute not found %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="117"/>
        <location filename="../src/PluginDescriptor.cpp" line="123"/>
        <location filename="../src/PluginDescriptor.cpp" line="129"/>
        <source>Required element not found %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="136"/>
        <source>Not valid value: &apos;%1&apos;, plugin: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="144"/>
        <source>Platform arch is unknown: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="151"/>
        <source>Platform bits is unknown: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="170"/>
        <source>Invalid depends token: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="229"/>
        <source>Plugin circular dependency detected: %1 &lt;-&gt; %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="313"/>
        <source>Can&apos;t satisfy dependencies for %1 !</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::RegisterServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="116"/>
        <source>Register &apos;%1&apos; service</source>
        <translation>Регистрация сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="124"/>
        <source>Service is already registered &apos;%1&apos;</source>
        <translation>Сервис уже зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="128"/>
        <source>Illegal service state: %1</source>
        <translation>Ошибка сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="134"/>
        <source>Only one service of specified type is allowed: %1</source>
        <translation>Разрешён только один экземпляр сервиса: %1</translation>
    </message>
</context>
<context>
    <name>U2::TaskSchedulerImpl</name>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="19"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="510"/>
        <source>New</source>
        <translation>Новая</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="19"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="512"/>
        <source>Prepared</source>
        <translation>Инициализирована</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="19"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="514"/>
        <source>Running</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="19"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="516"/>
        <source>Finished</source>
        <translation>Завершена</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="38"/>
        <source>Canceling task: %1</source>
        <translation>Отменяется задача &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="63"/>
        <source>Subtask {%1} is canceled</source>
        <translation>Подзадача {%1} отменена.</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="70"/>
        <source>Subtask {%1} is failed: %2</source>
        <translation>Подзадача {%1} завершена с ошибкой: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="86"/>
        <source>TimeOut Error. TimeOut = %1 sec.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="247"/>
        <source>Waiting for resource &apos;%1&apos;, count: %2</source>
        <translation>Ожидание ресурса &quot;%1&quot;, требуется: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="260"/>
        <source>Not enough resources for the task, resource name: &apos;%1&apos; max: %2%3 requested: %4%5</source>
        <translation>Не достаточно ресурсов для выполнения задачи: &apos;%1&apos; макс: %2%3	запрошено: %4%5</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="263"/>
        <source>Waiting for resource &apos;%1&apos;, count: %2%3</source>
        <translation>Ожидание ресурса &quot;%1&quot;, требуется: %2%3</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="381"/>
        <source>Registering new task: %1</source>
        <translation>Добавлена задача &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="434"/>
        <source>Uregistering task: %1</source>
        <translation>Задача удаляется из списка: &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="519"/>
        <source>Invalid name</source>
        <translation>Неизвестное состояние</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="720"/>
        <source>Deleting task: %1</source>
        <translation>Удаляется задача &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="580"/>
        <source>promoting task {%1} to &apos;%2&apos;</source>
        <translation>Задача {%1} %2.</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="582"/>
        <source>promoting task {%1} to &apos;%2&apos;, error &apos;%3&apos;</source>
        <translation>Задача {%1} %2; ошибка: %3</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="592"/>
        <source>Starting {%1} task</source>
        <translation>Старт задачи {%1}</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="618"/>
        <source>Task {%1} finished with error: %2</source>
        <translation>Задача {%1} завершена с ошибкой: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="620"/>
        <source>Task {%1} canceled</source>
        <translation>Отменяется задача {%1}</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="622"/>
        <source>Task {%1} finished</source>
        <translation>Задача {%1} завершена</translation>
    </message>
</context>
<context>
    <name>U2::UnregisterServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="236"/>
        <source>Service is not registered: %1</source>
        <translation>Сервис не зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="229"/>
        <source>Unregister &apos;%1&apos; service</source>
        <translation>Дерегистрация сервиса &quot;%1&quot;</translation>
    </message>
</context>
</TS>
