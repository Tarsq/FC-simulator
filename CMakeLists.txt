add_executable(FC2  FC2.c
                    pid.c
                    sh_mem.c
                    periodic.c
                    Gimbals_position.c
                    sensors.c
                    )

add_executable(model  model.c
                    sh_mem.c
                    first_order_plant.c
                    vis_logg.c
                    model_output.c)

add_executable(RC_Client  RC_Client.c
                    RC_Client.c)

target_link_libraries(FC2 pthread rt)
target_link_libraries(model pthread rt m)
target_link_libraries(RC_Client pthread rt)
