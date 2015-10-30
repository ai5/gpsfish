// NAME
//       filetostdout - Stream added files in a directory to stdout
//
// PREREQUISTE
//       1. https://github.com/go-fsnotify/fsnotify
//
// SYNOPSIS
//       filetostdout [option]...
//
// DESCRIPTION
//       This program watches a directory where new files are added by using
//       inotify on Linux or kqueue on Mac OS X. Whenever it detects a new
//       file, it prints out its conent to stdout.
//
//       The file name should be in a form of /\d{4}.txt/.
//
// OPTIONS
//       -d Specify a diretory to be monitored.
//
//       -s Specify an initial number for the file name sequence
//
//       -k Enable keep alive
//
// EXAMPLE
//
//       % filetostdout -d in -k
//
package main

import (
	"gopkg.in/fsnotify.v1"

	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	//"os/signal"
	"path"
	"strings"
	"sync"
	"time"
)

// Command line options
var pdirectory = flag.String("d", ".", "a directory to be monitored")
var pid = flag.Int("s", 0, "Initial number for the file name sequence")
var pkeepalive = flag.Bool("k", false, "enable keep alive")

// Global variables

var logger = log.New(os.Stderr, "", log.Ldate|log.Lmicroseconds)

// main
func main() {
	flag.Parse()
	var wg sync.WaitGroup

	// Monitor a directory
	watcher, err := fsnotify.NewWatcher()
	if err != nil {
		logger.Fatal(err)
	}
	defer watcher.Close()

	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			select {
			//case event := <-watcher.Events:
			case <-watcher.Events:
				//fmt.Println("event: ", event)
				//if event.Op&fsnotify.Create == fsnotify.Create {
				doScan()
				//}
			case err := <-watcher.Errors:
				logger.Println("ERROR: ", err)
			}
		}
	}()

	// Initial scan
	doScan()

	err = watcher.Add(*pdirectory)
	if err != nil {
		logger.Fatal(err)
	}

	// Prevent Ctrl+C
	/*
		c := make(chan os.Signal, 1)
		signal.Notify(c, os.Interrupt)
		firstSignal := false
		wg.Add(1)
		go func() {
			defer wg.Done()
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

	if *pkeepalive {
		for {
			select {
			case <-time.After(time.Second * 60):
				logPrint("(keepalive)")
				fmt.Println("")
			}
		}
	}

	wg.Wait()
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
	logger.Printf("%s R %04d %s", host(), *pid, msg)
}

func nextFileName() string {
	return fmt.Sprintf("%04d.txt", *pid)
}

func nextFilePath() string {
	return path.Join(*pdirectory, nextFileName())
}

func readAndPrint(file string) {
	dat, err := ioutil.ReadFile(file)
	if err != nil {
		logger.Println("ERROR: Failed to read a file ", file, err)
		return
	}

	str := strings.TrimSpace(string(dat))
	if str == "" {
		logger.Println("WARN: empty ", file)
		return
	}
	fmt.Println(str)
	logPrint(str)
	*pid++
}

func doScan() {
	for {
		file := nextFilePath()
		if _, err := os.Stat(file); err == nil {
			readAndPrint(file)
		} else {
			break
		}
	}
}

/* vim: set ts=4 sw=4 expandtab ft=go : */
