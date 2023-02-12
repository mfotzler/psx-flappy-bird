# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	FlappyBird
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "Flappy Bird"
	HOMEPAGE_URL "https://github.com/mfotzler/psx-flappy-bird"
)

psn00bsdk_add_executable(template GPREL main.c game.c)
psn00bsdk_target_incbin(template PRIVATE birdTexture assets/bird.tim)
psn00bsdk_target_incbin(template PRIVATE cloudTexture assets/cloud.tim)
psn00bsdk_target_incbin(template PRIVATE groundTexture assets/ground.tim)
psn00bsdk_target_incbin(template PRIVATE pipeTexture assets/pipe.tim)

psn00bsdk_add_cd_image(
	iso      # Target name
	template # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS template
)
