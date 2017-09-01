extern crate image;
extern crate json;

use std::fs::File;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::Path;
use std::process::{Command, Stdio};
use std::{thread, time};

use image::{GenericImage, Pixel, ImageBuffer};

fn main() {
    let mut child = Command::new("java")
            .args(&["-cp", "../sim/classes", "sobel.SobelFilter_test"])
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .spawn()
            .expect("failed to execute process");

    {
        let stdin = child.stdin.as_mut().unwrap();
        let mut stdout = BufReader::new(child.stdout.take().unwrap());
        let img = image::open(&Path::new("data/lena512.bmp")).unwrap();
        let (width, height) = img.dimensions();

        let write_thread = thread::spawn(move || {
            let mut buf = String::new();
            let mut img = ImageBuffer::new(width, height);
            for y in 0..height {
                for x in 0..width {
                    stdout.read_line(&mut buf).unwrap();
                    let value = json::parse(&buf).unwrap().pop().as_u8().unwrap();
                    buf.clear();
                    img.put_pixel(x, y, image::Luma([value]));
                }
                println!("y = {}", y);
            }
            img.save(&Path::new("data/out.png")).unwrap();
        });

        for (x, y, pixel) in img.pixels() {
            stdin.write(format!("[{}]\n", pixel.to_luma()[0]).as_bytes()).unwrap();
            stdin.flush().unwrap();
        }
        stdin.write(b"\n").unwrap();
        stdin.flush().unwrap();

        let res = write_thread.join();
        res.unwrap();
    }

    child.kill().expect("could not end simulation");
}
