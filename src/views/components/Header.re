[%bs.raw "require('./header.css')"];

open Types;

let component = ReasonReact.statelessComponent("Header");

let make = (~tabs: list(tab), ~selectedTab, _children) => {
  ...component,
  render: _self =>
    <div className="header">
      <div className="menu-items">
        (
          tabs
          |> Array.of_list
          |> Array.map(tab => {
               let selected = tab.id === selectedTab ? "selected" : "";
               <span className=("menu-item " ++ selected)>
                 (ReasonReact.string(tab.text))
               </span>;
             })
          |> ReasonReact.array
        )
      </div>
      <div className="logout-wrapper">
        <a href="#"> (ReasonReact.string("Logout")) </a>
      </div>
    </div>
};