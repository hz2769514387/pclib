cmake_minimum_required(VERSION 2.8.8)
project(LogicTest)

# -----------------------------------------------------------------------------
#检测操作系统，只支持 linux 和 windows
# -----------------------------------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(LP2_OS_LINUX 1)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(LP2_OS_WINDOWS 1)
	set(LP2_CPU_X86 0)
	set(LP2_CPU_X86_64 1)
else ()
	message(FATAL_ERROR "Unknown OS '${CMAKE_SYSTEM_NAME}'")
endif()


# -----------------------------------------------------------------------------
# 加入包含目录
# -----------------------------------------------------------------------------
SET(PCLib_INCLUDE_DIRECTORIES
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/openssl-1.0.1t/include
)
include_directories(${PCLib_INCLUDE_DIRECTORIES})

# -----------------------------------------------------------------------------
# 加入库文件
# -----------------------------------------------------------------------------
if(LP2_OS_LINUX)
        SET(LIBOPENSSL1_LIB "${CMAKE_SOURCE_DIR}/src/openssl-1.0.1t/lib_linux_x64/libssl.a")
        SET(LIBOPENSSL2_LIB "${CMAKE_SOURCE_DIR}/src/openssl-1.0.1t/lib_linux_x64/libcrypto.a")
	set(PCLIB_LIB
		${LIBOPENSSL1_LIB}
		${LIBOPENSSL2_LIB}
	)
	link_libraries(${PCLIB_LIB})  
endif()

# -----------------------------------------------------------------------------
# 加入源目录
# -----------------------------------------------------------------------------
aux_source_directory("${CMAKE_SOURCE_DIR}/src" PCLib_MAIN_SRC_DIRECTORIES)
aux_source_directory("${CMAKE_SOURCE_DIR}/src/pclib" PCLib_PCLIB_SRC_DIRECTORIES)
aux_source_directory("${CMAKE_SOURCE_DIR}/src/zlib-1.2.8" PCLib_PCLIBZLIB_SRC_DIRECTORIES)

SET(PCLib_SOURCE_DIRECTORIES
	${PCLib_MAIN_SRC_DIRECTORIES}
	${PCLib_PCLIB_SRC_DIRECTORIES}
	${PCLib_PCLIBZLIB_SRC_DIRECTORIES}
)

# -----------------------------------------------------------------------------
# 加预处理器定义 ,add_definitions(-D__LYNX)
# -----------------------------------------------------------------------------
if(LP2_OS_LINUX)
	
endif()

# -----------------------------------------------------------------------------
# 链接搜索路径(必须放在add_executable/ADD_LIBRARY前)
# -----------------------------------------------------------------------------
if (WIN32)
	SET(PCLib_LIB_DIRECTORIES
		${PROJECT_SOURCE_DIR}/src
	)
	link_directories(${PCLib_LIB_DIRECTORIES})
endif ()


# -----------------------------------------------------------------------------
# 生成路径
# -----------------------------------------------------------------------------
if(LP2_OS_LINUX)
	ADD_EXECUTABLE(LogicTest  ${PCLib_SOURCE_DIRECTORIES})
else ()
	ADD_EXECUTABLE(LogicTest  "${PCLib_SOURCE_DIRECTORIES}")
endif()
set_target_properties(LogicTest  PROPERTIES PREFIX "")

# -----------------------------------------------------------------------------
# 链接配置(必须放在add_executable/ADD_LIBRARY后)
# -----------------------------------------------------------------------------
if (WIN32)
	#加入MFC支持
	SET(CMAKE_MFC_FLAG 2)

	#设置静态链接C运行库
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	
	#请求管理员权限
	SET_TARGET_PROPERTIES(LogicTest PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
else ()
	#使用c++11
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif ()

# -----------------------------------------------------------------------------
# 链接参数(必须放在add_executable/ADD_LIBRARY后)
# -----------------------------------------------------------------------------
if(LP2_OS_LINUX)
	SET(PCLib_l_LIB_PARAMS
		pthread
		rt
		m
		dl
		g
		Wall
	)
	target_link_libraries(LogicTest ${PCLib_l_LIB_PARAMS})
endif()

# -----------------------------------------------------------------------------
# 设置输出目录，linux下你可以用以下cmake的命令生成code::blocks或eclipse的工程文件
#cmake .. -G "CodeBlocks - Unix Makefiles"	-DCMAKE_BUILD_TYPE=debug 
#cmake .. -G "Eclipse CDT4 - Unix Makefiles" 	-DCMAKE_BUILD_TYPE=debug 
# -----------------------------------------------------------------------------
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)



