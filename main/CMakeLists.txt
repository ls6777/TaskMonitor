idf_component_register(SRCS "main.cpp"
                            "Src/DefaultTask.cpp"
                            "Src/TargetPort.cpp"
                            "Src/Task1.cpp"
                            "Src/Task2.cpp"
                            "Src/Task3.cpp"
                            "Src/TaskMonitor.cpp"
                       INCLUDE_DIRS "Inc"
                       PRIV_REQUIRES esp_system freertos log)
