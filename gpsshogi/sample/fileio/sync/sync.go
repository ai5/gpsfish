package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"path"
	"sync"
	"time"
)

// Command line options
var plocal = flag.String("l", "local-csa", "a local directory")
var phost = flag.String("h", "komaba", "host name")
var premote = flag.String("r", "remote-csa", " a remote directory")
var prename = flag.Bool("N", false, "rename directory upon start")

// Global variables

var logger = log.New(os.Stderr, "", log.Ldate|log.Lmicroseconds)
var wg sync.WaitGroup

// main
func main() {
	flag.Parse()

	initializeLocal()
	initializeRemote()

	// Prevent Ctrl+C
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	firstSignal := false
	wg.Add(1)
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

	localToRemote()
	remoteToLocal()

	wg.Wait()
}

func initializeLocal() {
	logger.Printf("Set up a local directory...  %s\n", *plocal)
	if _, err := os.Stat(*plocal); err == nil && *prename {
		str := time.Now().Format(time.RFC3339)
		if err := os.Rename(*plocal, *plocal+"_"+str); err != nil {
			logger.Fatal(err)
		}
	}
	if err := os.MkdirAll(path.Join(*plocal, "in"), 0755); err != nil {
		logger.Fatal(err)
	}
	if err := os.MkdirAll(path.Join(*plocal, "out"), 0755); err != nil {
		logger.Fatal(err)
	}
}

func initializeRemote() {
	logger.Printf("Set up a remote directory...  %s\n", *premote)
	if *prename {
		now := time.Now().Format(time.RFC3339)
		ss := fmt.Sprintf("[ -d %[1]s ] && mv %[1]s %[2]s", *premote, *premote+"_"+now)
		sshCommand(ss)
	}

	s := fmt.Sprintf("mkdir -p %[1]s/in; mkdir -p %[1]s/out; chmod 755 %[1]s/in; chmod 755 %[1]s/out", *premote)
	sshCommand(s)
}

func sshCommand(s string) {
	cmd := exec.Command("ssh", *phost, s)
	if err := cmd.Run(); err != nil {
		logger.Fatal(err)
	}
}

func localToRemote() {
	wg.Add(1)
	go func() {
		for {
			logger.Println("Local to Remote... ")

			cmd := exec.Command("sh", "-c",
				fmt.Sprintf("filetostdout -k -d %s/in", *plocal))
			cmd.Stderr = os.Stderr
			ssh := exec.Command("sh", "-c",
				fmt.Sprintf("ssh -o ServerAliveInterval=20 -o ServerAliveCountMax=3 %s /home/gps/bin/stdintofile -d %s/in", *phost, *premote))
			ssh.Stderr = os.Stderr

			// pipe
			ssh.Stdin, _ = cmd.StdoutPipe()

			if err := ssh.Start(); err != nil {
				logger.Println(err)
			}
			if err := cmd.Start(); err != nil {
				logger.Println(err)
			}

			// Wait
			if err := ssh.Wait(); err != nil {
				logger.Println(err)
			}

			//
			logger.Println("Killing Local to Remote... ")
			if ssh.ProcessState != nil && !ssh.ProcessState.Exited() {
				ssh.Process.Kill()
				logger.Println("  ssh killed")
			}
			if cmd.ProcessState != nil && !cmd.ProcessState.Exited() {
				cmd.Process.Kill()
				logger.Println("  cmd killed")
			}
			time.Sleep(1000 * time.Millisecond)
		}
	}()
}

func remoteToLocal() {
	wg.Add(1)
	go func() {
		for {
			logger.Println("Remote to Local... ")

			ssh := exec.Command("sh", "-c",
				fmt.Sprintf("ssh -o ServerAliveInterval=20 -o ServerAliveCountMax=3 %s /home/gps/bin/filetostdout -k -d %s/out", *phost, *premote))
			ssh.Stderr = os.Stderr
			cmd := exec.Command("sh", "-c",
				fmt.Sprintf("stdintofile -d %s/out", *plocal))
			cmd.Stderr = os.Stderr

			// pipe
			cmd.Stdin, _ = ssh.StdoutPipe()

			if err := cmd.Start(); err != nil {
				logger.Println(err)
			}
			if err := ssh.Start(); err != nil {
				logger.Println(err)
			}

			// Wait to finish
			if err := ssh.Wait(); err != nil {
				logger.Println(err)
			}

			logger.Println("Killing Remote to Local... ")
			if ssh.ProcessState != nil && !ssh.ProcessState.Exited() {
				ssh.Process.Kill()
				logger.Println("  ssh killed")
			}
			if cmd.ProcessState != nil && !cmd.ProcessState.Exited() {
				cmd.Process.Kill()
				logger.Println("  cmd killed")
			}
			time.Sleep(1000 * time.Millisecond)
		}
	}()
}

/* vim: set ts=4 sw=4 expandtab ft=go : */
