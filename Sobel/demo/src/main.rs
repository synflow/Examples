extern crate image;

use std::fs::File;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::Path;
use std::process::{Command, Stdio};
use std::thread;

use image::{GenericImage, Pixel};

fn main() {
    let mut child = Command::new("java")
            .args(&["-cp", "../sim/classes", "sobel.SobelFilterTest"])
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .spawn()
            .expect("failed to execute process");

    let stdin = child.stdin.as_mut().unwrap();
    let mut stdout = BufReader::new(child.stdout.take().unwrap());

    let child = thread::spawn(move || {
        // some work here
        let mut buf = String::new();
        println!("reading from output");
        stdout.read_line(&mut buf).unwrap();
        println!("read line: {}", buf);
    });

    let img = image::open(&Path::new("data/lena512.bmp")).unwrap();
    for (x, y, pixel) in img.pixels() {
        stdin.write(format!("[{}]\n", pixel.to_luma()[0]).as_bytes()).unwrap();
        stdin.flush().unwrap();
    }
    stdin.write(b"\n").unwrap();
    stdin.flush().unwrap();

    let res = child.join();
    res.unwrap();
}
