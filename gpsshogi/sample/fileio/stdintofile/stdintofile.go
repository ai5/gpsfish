// NAME
//       stdintofile - Absorb lines from stdin and write them to seprate files.
//
// PREREQUISTE
//
// SYNOPSIS
//       stdintofile [option]...
//
// DESCRIPTION
//       This program receives lines from stdin and writes them to seperate
//       files with incremental numbering names.
//
//       The file name should be in a form of /\d{4}.txt/.
//
// OPTIONS
//       -d Specify a diretory to be monitored.
//
//       -s Specify an initial number for the file name sequence
//
// EXAMPLE
//
//       % stdintofile -d out -s 0
//
package main

import (
	"bufio"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	//"os/signal"
	"path"
	"strings"
	"sync"
)

// Command line options
var pdirectory = flag.String("d", ".", "a directory to be monitored")
var pid = flag.Int("s", 0, "Initial number for the file name sequence")

// Global variables

var logger = log.New(os.Stderr, "", log.Ldate|log.Lmicroseconds)

// main
func main() {
	flag.Parse()
	mkdir(*pdirectory)

	/*
		// Prevent Ctrl+C
		c := make(chan os.Signal, 1)
		signal.Notify(c, os.Interrupt)
		firstSignal := false
		go func() {
			for sig := range c {
				logger.Printf("trapped %v. Do again to stop.", sig)
				if firstSignal {
					os.Exit(1)
				} else {
					firstSignal = true
				}
			}
		}()
	*/

	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := scanner.Text()
		if text == "" {
			logger.Println("(nop)")
		} else {
			writeln(text)
		}
	}
}

func writeln(s string) {
	s += "\n"
	tmp, _ := ioutil.TempFile("", "gps")
	defer os.Remove(tmp.Name())

	tmp.WriteString(s)
	tmp.Close()

	os.Rename(tmp.Name(), nextFilePath())
	logPrint(s)
	*pid++
}

// hosts returns a host name
func host() string {
	var once sync.Once
	name := ""
	once.Do(func() {
		name, _ = os.Hostname()
		if i := strings.Index(name, "."); i >= 0 {
			name = name[0:i]
		}
	})
	return fmt.Sprintf("%8s", name)
}

// logPrint outputs log messages for the local host
func logPrint(msg string) {
	logger.Printf("%s W %04d %s", host(), *pid, msg)
}

func nextFileName() string {
	return fmt.Sprintf("%04d.txt", *pid)
}

func nextFilePath() string {
	return path.Join(*pdirectory, nextFileName())
}

func mkdir(path string) {
	fileInfo, err := os.Stat(path)
	if err != nil {
		os.Mkdir(path, 0755)
	} else if !fileInfo.IsDir() {
		logger.Fatal("There already exists a file: " + path)
	}
}

/* vim: set ts=4 sw=4 expandtab ft=go : */
