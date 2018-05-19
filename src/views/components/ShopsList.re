let component = ReasonReact.statelessComponent("shops");

open Types;

let make = (~data, ~btns, ~clickHandler, _children) => {
  ...component,
  render: _self =>
    data
    |> Array.of_list
    |> Array.map((shop: shop) =>
         <div key=shop.id className="shop-wrapper">
           <p> (ReasonReact.string(shop.name)) </p>
           <img src=shop.picture />
           <div className="btns-wrapper">
             (
               btns
               |> Array.of_list
               |> Array.map(btn =>
                    <button
                      className=("shop-btn " ++ btn ++ "-btn")
                      onClick=(clickHandler(shop.id, btn))>
                      (btn |> String.capitalize |> ReasonReact.string)
                    </button>
                  )
               |> ReasonReact.array
             )
           </div>
         </div>
       )
    |> ReasonReact.array
};